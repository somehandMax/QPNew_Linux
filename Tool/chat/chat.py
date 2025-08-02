import os
import sys
import socket
import threading
import configparser
import time
import queue
from getpass import getpass

# 获取本机IP地址
def get_local_ip():
    s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    try:
        # 连接到一个公共DNS服务器（不实际发送数据）
        s.connect(('8.8.8.8', 80))
        ip = s.getsockname()[0]
    except Exception:
        ip = '127.0.0.1'
    finally:
        s.close()
        return ip

# 检查配置文件
def check_config():
    config = configparser.ConfigParser()
    
    # 检查check.ini是否存在
    if not os.path.exists('./Tool/chat/check.ini'):
        config['DEFAULT'] = {'first_run': '0'}
        with open('./Tool/chat/check.ini', 'w') as configfile:
            config.write(configfile)
    
    config.read('./Tool/chat/check.ini')
    
    # 如果是第一次运行
    if config['DEFAULT'].get('first_run', '0') == '0':
        print("欢迎使用局域网聊天室！这是您第一次使用，请设置用户名。")
        username = input("请输入用户名: ")
        
        # 保存用户名
        with open('./Tool/chat/name.txt', 'w') as f:
            f.write(username)
        
        # 更新配置
        config['DEFAULT']['first_run'] = '1'
        with open('./Tool/chat/check.ini', 'w') as configfile:
            config.write(configfile)
        
        print(f"用户名 '{username}' 已保存！")
    
    # 读取用户名
    if not os.path.exists('./Tool/chat/name.txt'):
        # 如果name.txt不存在，创建一个默认用户名
        username = "用户" + str(int(time.time()) % 1000)
        with open('./Tool/chat/name.txt', 'w') as f:
            f.write(username)
    else:
        with open('./Tool/chat/name.txt', 'r') as f:
            username = f.read().strip()
            if not username:
                # 如果用户名为空，创建一个默认用户名
                username = "用户" + str(int(time.time()) % 1000)
                with open('./Tool/chat/name.txt', 'w') as f:
                    f.write(username)
    
    return username

# 修改用户名
def change_username():
    current_username = check_config()
    print(f"\n当前用户名: {current_username}")
    new_username = input("请输入新用户名: ").strip()
    
    if not new_username:
        print("【系统】用户名不能为空")
        return current_username
    
    # 保存新用户名
    with open('./Tool/chat/name.txt', 'w') as f:
        f.write(new_username)
    
    print(f"【系统】用户名已修改为: {new_username}")
    return new_username

# 获取服务器名称
def get_server_name():
    if os.path.exists('./Tool/chat/server_name.txt'):
        with open('./Tool/chat/server_name.txt', 'r') as f:
            name = f.read().strip()
            return name if name else "一个没有名字的服务器"
    return "一个没有名字的服务器"

# 设置服务器名称
def set_server_name(name):
    with open('./Tool/chat/server_name.txt', 'w') as f:
        f.write(name)

# UDP广播服务器
def broadcast_server(server_name, password_protected, stop_event):
    broadcast_port = 50000
    server_port = 50001
    local_ip = get_local_ip()
    
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.setsockopt(socket.SOL_SOCKET, socket.SO_BROADCAST, 1)
    sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    
    print(f"【广播服务】已启动 (IP: {local_ip})")
    
    try:
        while not stop_event.is_set():
            # 发送广播消息: CHAT_SERVER|名称|IP|端口|是否需要密码
            message = f"CHAT_SERVER|{server_name}|{local_ip}|{server_port}|{1 if password_protected else 0}"
            sock.sendto(message.encode(), ('<broadcast>', broadcast_port))
            time.sleep(2)  # 每2秒广播一次
    except Exception as e:
        print(f"广播出错: {e}")
    finally:
        sock.close()

# 接收广播寻找服务器
def discover_servers(stop_event):
    broadcast_port = 50000
    servers = []
    seen_servers = set()
    
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.setsockopt(socket.SOL_SOCKET, socket.SO_BROADCAST, 1)
    sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    sock.bind(('', broadcast_port))
    sock.settimeout(3)  # 3秒超时
    
    print("正在搜索局域网中的服务器...")
    
    start_time = time.time()
    while time.time() - start_time < 5 and not stop_event.is_set():  # 搜索5秒
        try:
            data, addr = sock.recvfrom(1024)
            message = data.decode()
            
            if message.startswith("CHAT_SERVER|"):
                parts = message.split('|')
                if len(parts) >= 5:
                    server_name = parts[1]
                    server_ip = parts[2]
                    server_port = parts[3]
                    requires_password = parts[4] == '1'
                    
                    # 避免重复添加
                    if server_ip not in seen_servers:
                        seen_servers.add(server_ip)
                        servers.append((server_name, server_ip, server_port, requires_password))
                        print(f"发现服务器: {server_name} ({server_ip}:{server_port}, {'需要密码' if requires_password else '无需密码'})")
        except socket.timeout:
            continue
        except Exception as e:
            print(f"接收广播出错: {e}")
    
    sock.close()
    return servers

# 服务器端TCP处理
def start_server(server_name, password_protected, password, username):
    server_port = 50001
    server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    
    # 创建输入队列
    input_queue = queue.Queue()
    
    # 用于跟踪当前连接的用户名
    # 重要修复：将服务器自身的用户名加入connected_usernames
    connected_usernames = [username]
    
    # 状态变量，用于处理连接确认
    waiting_for_approval = [False]
    current_approval = [None]  # (conn, addr, final_username)
    approval_lock = threading.Lock()
    
    try:
        server_socket.bind(('', server_port))
        server_socket.listen(5)
        print(f"【服务器】'{server_name}' 已启动，等待客户端连接... (端口: {server_port})")
        print(f"【系统】服务器管理员: {username}")
        
        clients = []  # 存储所有客户端连接
        
        # 生成唯一用户名（第一个用户用原名，后续加1,2,3...）
        def get_unique_username(proposed_name):
            # 检查是否有完全相同的用户名
            if proposed_name not in connected_usernames:
                return proposed_name
            
            # 检查是否有带数字后缀的用户名
            suffix = 1
            while True:
                new_name = f"{proposed_name}{suffix}"
                if new_name not in connected_usernames:
                    return new_name
                suffix += 1
        
        # 踢出用户
        def kick_user(target_username):
            if target_username == username:
                print("【系统】不能踢出自己")
                return False
            
            # 查找目标用户
            for i, client_username in enumerate(connected_usernames):
                if client_username == target_username and i > 0:  # i>0 排除服务器自身
                    # 获取对应的连接
                    if i-1 < len(clients):  # i-1 因为connected_usernames包含服务器
                        conn = clients[i-1]
                        try:
                            # 发送踢出消息
                            conn.send(b"KICKED")
                            # 关闭连接
                            conn.close()
                            # 从列表中移除
                            connected_usernames.pop(i)
                            clients.pop(i-1)
                            # 通知所有用户
                            kick_message = f"【系统】用户 {target_username} 被管理员踢出聊天室"
                            for client in clients:
                                try:
                                    client.send(kick_message.encode())
                                except:
                                    clients.remove(client)
                            print(f"【系统】已踢出用户: {target_username}")
                            return True
                        except Exception as e:
                            print(f"踢出用户出错: {e}")
                            return False
            
            print(f"【系统】找不到用户: {target_username}")
            return False
        
        # 专门处理用户输入的线程
        def input_handler():
            while True:
                try:
                    message = input()
                    
                    # 检查是否正在等待连接确认
                    with approval_lock:
                        if waiting_for_approval[0] and current_approval[0] is not None:
                            conn, addr, final_username = current_approval[0]
                            decision = message.strip().lower()
                            
                            if decision == '' or decision == 'y':
                                conn.send(b"APPROVED")
                                print(f"已允许客户端 {addr[0]}（用户名: {final_username}）连接")
                                
                                # 发送欢迎消息
                                conn.send(f"【系统】欢迎 {final_username} 加入聊天室！".encode())
                                
                                # 添加到客户端列表
                                clients.append(conn)
                                connected_usernames.append(final_username)
                                
                                # 通知所有用户新用户加入
                                join_message = f"【系统】{final_username} 已加入聊天室"
                                for client in clients:
                                    if client != conn:
                                        try:
                                            client.send(join_message.encode())
                                        except:
                                            clients.remove(client)
                                
                                # 启动消息处理线程
                                threading.Thread(
                                    target=handle_client, 
                                    args=(conn, addr, final_username), 
                                    daemon=True
                                ).start()
                                
                                # 重置状态
                                waiting_for_approval[0] = False
                                current_approval[0] = None
                            elif decision == 'n':
                                conn.send(b"REJECTED")
                                conn.close()
                                print(f"已拒绝客户端 {addr[0]}（用户名: {final_username}）连接")
                                
                                # 重置状态
                                waiting_for_approval[0] = False
                                current_approval[0] = None
                            else:
                                # 无效输入，重新提示
                                print("【系统】无效输入，请按回车或输入 'y' 表示同意，'n' 表示拒绝")
                        else:
                            # 处理特殊命令
                            if message.startswith("/kick "):
                                target_username = message[6:].strip()
                                kick_user(target_username)
                            elif message == "/users":
                                print("\n当前在线用户:")
                                for i, user in enumerate(connected_usernames):
                                    prefix = "* " if i > 0 else "(你) "
                                    print(f"{prefix}{user}")
                                print()
                            else:
                                # 将输入放入队列，作为聊天消息
                                input_queue.put(message)
                except Exception as e:
                    print(f"输入处理出错: {e}")
                    break
        
        # 启动输入处理线程
        input_thread = threading.Thread(target=input_handler, daemon=True)
        input_thread.start()
        
        # 处理客户端消息的线程
        def handle_client(conn, addr, client_username):
            try:
                # 接收并广播消息
                while True:
                    data = conn.recv(1024)
                    if not data:
                        break
                    
                    message = data.decode()
                    
                    # 为客户端消息添加"*"前缀，以便与服务器自己发送的消息区分
                    print(f"*{message}")
                    
                    # 广播给所有客户端（除了发送者）
                    for client in clients:
                        if client != conn:
                            try:
                                client.send(data)
                            except:
                                clients.remove(client)
            except Exception as e:
                print(f"客户端处理出错: {e}")
            finally:
                if client_username in connected_usernames:
                    connected_usernames.remove(client_username)
                if conn in clients:
                    clients.remove(conn)
                conn.close()
                print(f"客户端 {client_username} ({addr}) 已断开连接")
        
        # 服务器端发送消息的线程
        def server_send_thread():
            while True:
                try:
                    # 从队列获取消息
                    message = input_queue.get()
                    
                    # 检查是否是退出命令
                    if message.lower() == '/exit':
                        # 通知所有客户端服务器即将关闭
                        for client in clients:
                            try:
                                client.send("【系统】服务器即将关闭，聊天室结束".encode())
                            except:
                                pass
                        os._exit(0)
                    
                    # 检查是否为空白消息
                    if not message.strip():
                        print("【系统】不能发送空白消息，请输入有效内容")
                        continue
                    
                    # 服务器发送的消息格式与其他用户一致
                    full_message = f"{username}: {message}"
                    # 服务器自己发送的消息不加"*"，直接显示
                    print(full_message)
                    
                    # 广播给所有客户端
                    for client in clients:
                        try:
                            client.send(full_message.encode())
                        except:
                            clients.remove(client)
                except Exception as e:
                    print(f"发送消息出错: {e}")
                    break
        
        # 启动服务器发送消息的线程
        send_thread = threading.Thread(target=server_send_thread, daemon=True)
        send_thread.start()
        print("【系统】您现在可以输入消息了（输入'/exit'退出服务器）")
        print("【系统】输入 '/users' 查看当前在线用户，输入 '/kick 用户名' 踢出用户")
        
        # 处理连接请求
        while True:
            conn, addr = server_socket.accept()
            print(f"收到连接请求: {addr[0]}")
            
            # 先检查用户名
            try:
                username_check = conn.recv(1024).decode()
                if username_check.startswith("CHECK_USERNAME|"):
                    proposed_name = username_check.split("|")[1]
                    final_username = get_unique_username(proposed_name)
                    conn.send(f"USERNAME_OK|{final_username}".encode())
                    print(f"客户端 {addr[0]} 的用户名检查: {proposed_name} -> {final_username}")
                    
                    # 等待客户端确认收到用户名
                    conn.recv(1024)  # 接收确认消息
                else:
                    # 如果没有收到用户名检查，可能是旧版客户端
                    proposed_name = f"用户{len(clients)+1}"
                    final_username = get_unique_username(proposed_name)
                    conn.send(f"USERNAME_OK|{final_username}".encode())
                    print(f"客户端 {addr[0]} 使用默认用户名: {final_username}")
                    
                    # 等待客户端确认收到用户名
                    conn.recv(1024)  # 接收确认消息
            except Exception as e:
                print(f"用户名检查出错: {e}")
                conn.close()
                continue
            
            # 发送服务器状态
            if password_protected:
                conn.send(b"PASSWORD_REQUIRED")
                try:
                    client_password = conn.recv(1024).decode()
                    if client_password == password:
                        conn.send(b"APPROVED")
                        print(f"客户端 {addr[0]}（用户名: {final_username}）通过密码验证")
                        
                        # 发送欢迎消息
                        conn.send(f"【系统】欢迎 {final_username} 加入聊天室！".encode())
                        
                        # 添加到客户端列表
                        clients.append(conn)
                        connected_usernames.append(final_username)
                        
                        # 通知所有用户新用户加入
                        join_message = f"【系统】{final_username} 已加入聊天室"
                        for client in clients:
                            if client != conn:
                                try:
                                    client.send(join_message.encode())
                                except:
                                    clients.remove(client)
                        
                        # 启动消息处理线程
                        threading.Thread(
                            target=handle_client, 
                            args=(conn, addr, final_username), 
                            daemon=True
                        ).start()
                    else:
                        conn.send(b"REJECTED")
                        conn.close()
                        print(f"客户端 {addr[0]}（用户名: {final_username}）密码验证失败")
                except Exception as e:
                    print(f"密码验证出错: {e}")
                    conn.close()
            else:
                # 无密码服务器，需要等待管理员确认
                conn.send(b"WAITING_APPROVAL")
                
                # 设置连接确认状态
                with approval_lock:
                    waiting_for_approval[0] = True
                    current_approval[0] = (conn, addr, final_username)
                
                print(f"【系统】有新的连接请求来自 {addr[0]}（用户名: {final_username}），是否允许连接？(按回车或输入'y'表示同意，'n'表示拒绝)")
    
    except Exception as e:
        print(f"服务器错误: {e}")
    finally:
        server_socket.close()

# 客户端连接服务器
def connect_to_server(server_name, server_ip, server_port, requires_password, password, username):
    client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    
    try:
        client_socket.connect((server_ip, int(server_port)))
        print(f"成功连接到服务器 '{server_name}' ({server_ip}:{server_port})")
        
        # 先发送用户名检查请求
        client_socket.send(f"CHECK_USERNAME|{username}".encode())
        response = client_socket.recv(1024).decode()
        
        if response.startswith("USERNAME_OK|"):
            final_username = response.split("|")[1]
            print(f"【系统】您的用户名: {final_username}")
            username = final_username
            # 发送确认
            client_socket.send(b"USERNAME_CONFIRMED")
        else:
            print("【系统】用户名检查失败")
            client_socket.close()
            return False
        
        # 检查服务器要求
        status = client_socket.recv(1024).decode()
        
        if status == "PASSWORD_REQUIRED" and requires_password:
            if password is None:
                password = getpass("服务器需要密码，请输入: ")
            client_socket.send(password.encode())
            
            response = client_socket.recv(1024).decode()
            if response != "APPROVED":
                print("【系统】密码错误，无法连接到服务器")
                client_socket.close()
                return False
        elif status == "WAITING_APPROVAL":
            print("【系统】等待服务器管理员批准...")
            response = client_socket.recv(1024).decode()
            if response != "APPROVED":
                print("【系统】服务器管理员拒绝了您的连接请求")
                client_socket.close()
                return False
        
        print("【系统】已成功加入聊天室！输入消息开始聊天（输入'/exit'退出）")
        
        # 接收消息的线程
        def receive_messages():
            while True:
                try:
                    data = client_socket.recv(1024)
                    if not data:
                        break
                    
                    message = data.decode()
                    
                    # 检查是否是被踢出的消息
                    if message == "KICKED":
                        print("【系统】您已被管理员踢出聊天室")
                        os._exit(0)
                    
                    # 检查是否是自己的消息（服务器广播回来的）
                    if message.startswith(f"{username}: "):
                        # 这是自己发送的消息，直接显示（不加*）
                        print(message)
                    # 检查是否是系统消息
                    elif message.startswith("【系统】"):
                        print(message)
                    else:
                        # 这是其他用户的消息，在消息前添加"*"
                        print(f"*{message}")
                        
                except Exception as e:
                    print(f"接收消息出错: {e}")
                    break
            print("【系统】与服务器的连接已断开")
            os._exit(0)
        
        # 启动接收消息的线程
        threading.Thread(target=receive_messages, daemon=True).start()
        
        # 发送消息
        while True:
            message = input()
            
            # 检查是否是退出命令
            if message.lower() == '/exit':
                break
            
            # 检查是否为空白消息
            if not message.strip():
                print("【系统】不能发送空白消息，请输入有效内容")
                continue
            
            full_message = f"{username}: {message}"
            try:
                client_socket.send(full_message.encode())
            except Exception as e:
                print(f"发送消息出错: {e}")
                break
        
        client_socket.close()
        return True
    
    except Exception as e:
        print(f"连接错误: {e}")
        client_socket.close()
        return False

def main():
    # 检查配置并获取用户名
    username = check_config()
    print(f"当前用户名: {username}")
    
    # 选择模式
    print("\n请选择模式:")
    print("1. 服务器端")
    print("2. 客户端")
    print("3. 修改用户名")
    mode = input("输入选项: ").strip()
    
    if mode == '3':
        # 修改用户名
        username = change_username()
        # 重新显示主菜单
        print("\n")
        print("局域网聊天室 v1.0\n")
        print("本程序Max开发，bug反馈、交流请发邮箱：maxnb666@qq.com")
        main()
        return
    
    if mode == '1':
        # 服务器端
        print("\n请选择是否设置密码:")
        print("1. 设置密码")
        print("2. 不设置密码")
        password_option = input("输入选项: ").strip()
        
        password_protected = (password_option == '1')
        password = None
        
        if password_protected:
            password = getpass("请输入服务器密码: ")
            print("密码已设置")
        
        # 设置服务器名称
        current_name = get_server_name()
        print(f"\n当前服务器名称: {current_name}")
        print("是否要修改服务器名称？(y/n)")
        change_name = input().strip().lower()
        if change_name == 'y':
            new_name = input("请输入新的服务器名称: ").strip()
            if new_name:
                set_server_name(new_name)
                server_name = new_name
            else:
                print("【系统】名称不能为空，使用默认名称")
                set_server_name("一个没有名字的服务器")
                server_name = "一个没有名字的服务器"
        else:
            server_name = current_name
        
        print(f"【系统】服务器名称设置为: {server_name}")
        
        # 启动广播
        stop_event = threading.Event()
        broadcast_thread = threading.Thread(target=broadcast_server, args=(server_name, password_protected, stop_event), daemon=True)
        broadcast_thread.start()
        print(f"【系统】广播服务已启动，其他客户端可以发现此服务器 '{server_name}'")
        
        try:
            # 启动服务器
            start_server(server_name, password_protected, password, username)
        except KeyboardInterrupt:
            print("\n【系统】正在关闭服务器...")
            stop_event.set()
            broadcast_thread.join(timeout=1)
            print("【系统】服务器已关闭")
    
    elif mode == '2':
        # 客户端
        stop_event = threading.Event()
        
        # 搜索服务器
        servers = discover_servers(stop_event)
        
        if not servers:
            print("【系统】未发现可用服务器，请确保有服务器正在运行")
            return
        
        # 选择服务器
        print("\n请选择要连接的服务器:")
        for i, (name, ip, port, requires_password) in enumerate(servers, 1):
            print(f"{i}. {name} ({ip}:{port}) ({'需要密码' if requires_password else '无需密码'})")
        
        choice = int(input("输入服务器编号: ").strip()) - 1
        if choice < 0 or choice >= len(servers):
            print("【系统】无效的选择")
            return
        
        server_name, server_ip, server_port, requires_password = servers[choice]
        print(f"【系统】您选择了服务器: {server_name} ({server_ip}:{server_port})")
        
        # 连接服务器
        password = None
        if requires_password:
            password = getpass("服务器需要密码，请输入: ")
        
        connect_to_server(server_name, server_ip, server_port, requires_password, password, username)
    
    else:
        print("【系统】无效的选项")

if __name__ == "__main__":
    print("局域网聊天室 v1.0\n")
    print("本程序Max开发，bug反馈、交流请发邮箱：maxnb666@qq.com")
    main()