#include"QPNewHead.h"
int color_loading=0;
bool start=false,admin=false;
signed main();
void GetSystemHost(){
    // 定义一个字符数组name，用于存储主机名，数组大小为256字节
    char name[256];
    // 调用gethostname函数获取系统主机名，并将其存储在name数组中
    gethostname(name, sizeof(name));
    int sock_fd, intrface;
    struct ifreq buf[INET_ADDRSTRLEN];
    struct ifconf ifc;
    char *local_ip = NULL;
    int status = RUN_FAIL;
    if ((sock_fd = socket(AF_INET, SOCK_DGRAM, 0)) >= 0){
        ifc.ifc_len = sizeof(buf);
        ifc.ifc_buf = (caddr_t)buf;
        if (!ioctl(sock_fd, SIOCGIFCONF, (char *)&ifc)){
            intrface = ifc.ifc_len/sizeof(struct ifreq);
            while (intrface-->0){
                if(!(ioctl(sock_fd, SIOCGIFADDR, (char *)&buf[intrface]))){
                    local_ip = NULL;
                    local_ip = inet_ntoa(((struct sockaddr_in*)(&buf[intrface].ifr_addr))->sin_addr);
                    if(local_ip)break;
                }
            }
        }
        close(sock_fd);
    }
    // 输出系统主机名
    printf("\n您的系统名称为:%s\n您的IP地址为:%s\n", name, local_ip);
}
void PrintTitle(){
    // 使用原始字符串字面量输出标题栏图案
    printf(R"(                 0000000  0000000  00     0  0000000  0       0       0
                 0     0  0     0  00     0  0         0      0      0
                 0     0  0     0  0  0   0  0          0     0     0
                 0     0  0000000  0   0  0  0000000     0    0    0
                 0   0 0  0        0    0 0  0            0   0   0
                 0    00  0        0     00  0             0  0  0
                 00000000 0        0      0  0              0 0 0    
                       0  0        0      0  0000000          0
QPNew-Linux,由Max开发，bug反馈：maxnb666@qq.com)");
printf("\n");
}
void Saying(){
    // 初始化随机数种子
    srand(static_cast<unsigned int>(time(0)));
    // 创建一个包含句子的数组
    string sentences[]={
        "活在这珍贵的人间，太阳强烈，水波温柔。",
        "吸引一个人的最好方式是:独立自主 而非刻意谄媚。",
        "无论空中的月亮如何皎洁，倘若池塘是浑浊的，月亮就无法在水中映现出来。",
        "人世冷暖，如同盲人摸象，永远无法得知全貌，我只得用所有的真诚和勇气来探究它的虚实。",
        "无论多少人以过来人的口吻告诉我，这个世界远比你想象中更加肮脏险恶，我仍然坚持尽我最强的意念去相信它的光明和仁慈。",
        "庸人自扰。",
        "很不幸的是，任何一种负面的生活都能产生很多烂七八糟的细节，使它变得蛮有趣的，\n人就是在这种有趣中沉沦下去，从根本上忘记了这种生活需要改进。",
        "生活不可能像你想象得那么好，但也不会像你想象得那么糟。",
        "被时间的洪水淘过，最终仍然堆在一起的是同样材质的小石头。",
        "人类的悲欢并不相通，我只觉得他们吵闹。",
        "每个人都睁着眼睛，但不等于每个人都在看世界，许多人几乎不用自己的眼睛看，\n她们只听别人说，他们看到的世界永远是别人说的样子。",
        "天生我材必有用，千金散尽还复来。",
        "我的不幸，恰恰在于我缺乏拒绝的能力。我害怕一旦拒绝别人，便会在彼此的心里留下永远不可愈合的裂痕。",
        "珍惜每一个当下，毕竟他们都会成为曾经。",
        "君子在下则排一方之难，在上则息万物之嚣。",
        "不会有到不了的明天。",
        "我们不肯探索自己本身的价值，我们过分看重他人在自己生命里的参与。\n于是，孤独不再美好，失去了他人，我们惶惑不安。",
        "心存希翼，目有繁星；追光而遇，沐光而行。",
        "人生海海，山山而川，不过尔尔。",
        "花繁柳密树，拨得开，才是手段；风狂雨急时,立得定，方见脚跟。",
        "未经世故的人,习于顺境,反而苛以待人;饱经世故的人,深谙逆境反而宽以处世。"
    };
    // 获取句子数组的大小
    size_t numSentences=sizeof(sentences)/sizeof(sentences[0]);
    // 生成一个随机索引
    int randomIndex=rand()%numSentences;
    // 输出随机选择的句子
    printf("%s\n",sentences[randomIndex].c_str());
}
void ControlTerminal(string input){
    // 执行命令，并读取其输出
    FILE* pipe=popen(input.c_str(),"r");
    char buffer[128];
    // 输出命令的输出
    while(fgets(buffer,sizeof(buffer),pipe)!=NULL)cout<<buffer;
    // 关闭管道
    pclose(pipe);
}
void adb(){
	cl;
    string input,adbPath="./Tool/adb/adb";
    printf("直接输入adb指令即可，如 devices；输入exit退出");
    while(true){
        // 输出提示信息，提示用户输入adb命令
        printf("adb> ");
        // 读取用户输入的一行命令，并存储在input变量中
        getline(cin,input);
		if(input=="exit"){
            cl;
            break;
        } 
        // 将adb工具的路径和用户输入的命令拼接起来，形成完整的命令字符串
        input=adbPath+" "+input;
        // 调用ControlTerminal函数，执行拼接后的命令，并将命令的输出显示在控制台上
        ControlTerminal(input);
    }
}
void Tool(){
    printf("1.adb工具\n");
    printf("2.聊天室\n");
    char FunctionChoice;
    // 读取用户的选择
    cin>>FunctionChoice;
    // 如果用户选择执行adb命令，则调用adb函数
    if(FunctionChoice=='1')adb();
    else if(FunctionChoice=='2')system("./Tool/chat/chat");
}
void Game(){
    printf("4.2048\n");
    char FunctionChoice;
    // 读取用户的选择
    cin>>FunctionChoice;
    // 如果用户选择执行Tool命令，则调用Tool函数
    if(FunctionChoice=='4')system("./Game/2048");
}
void Color(){
    cout<<"背景颜色序号(0~7)？ 0-黑色,1-红色,2-绿色,3-黄色,4-蓝色,5-洋红色,6-青色,7-白色,8-默认 "<<endl;
    cout<<">>";
    int _color;// 定义一个整型变量_color，用于存储用户输入的颜色代码
    string enter;
    cin>>_color;// 读取用户输入的颜色代码，并存储在_color变量中
    cout<<"修改的颜色在下次生效"<<endl;
    sleep(3);
    fstream file("./bin/setting.txt", ios::in|ios::out);
    if (file.is_open()) {
        string firstLine;
        vector<string>lines;
        while(getline(file, firstLine)) {// 读取所有行
            lines.push_back(firstLine);
        }
        if (!lines.empty()){// 修改第一行最后一个字符
            if (!lines[0].empty()) {
                lines[0][lines[0].length()-1] = '0'+_color;
            }
        }
        file.close();// 写回文件
        file.open("./bin/setting.txt", ios::out | ios::trunc);
        for (const auto& line : lines) {
            file << line << endl;
        }
        file.close();
    }
    cl;
}
void loading(){//初始化载入数据
    ifstream file("./bin/setting.txt", ios::binary|ios::in);
    if (!file.is_open()) {
        cout << "文件打开失败" << endl;
        return;
    }
    string line;
    int count=0;
    while (getline(file, line)) {
        if(count==0)color_loading=line.back()-'0';
    }
    file.close();
}
void AskAdmin(){//向用户所要管理员权限
    cout<<"请输入管理员密码"<<endl;
    system("sudo -v -S");
    admin=true;//表示已获得管理员权限
    return;
}
signed main(){
    if(start==false){
        loading();
        if(color_loading<8&&color_loading>0)cout<<"\033[37;4"<<color_loading<<"m";
        start=true;
    }
    PrintTitle();// 输出标题栏
    GetSystemHost();// 获取系统主机名与IP地址
    Saying();// 输出一句一言
    printf("请选择您要执行的功能:\n");
    printf("color.颜色\n");
    printf("website.跳转网站\n");
    printf("game.游戏整合\n");
    printf("tool.工具整合\n");
    printf("sudo.索要管理员权限\n");
    printf("clear.清屏\n");
    puts("");
A:
    if(admin==true)printf("root ");
    printf(">> ");
    string FunctionChoice;
    getline(cin,FunctionChoice);// 读取用户的选择
    if(FunctionChoice=="color"){
        cl;
        Color();
    }
    else if(FunctionChoice=="website"){//跳转网站
        cl;
        system("./Other/Website_redirect");
    }
    else if(FunctionChoice=="game"){//游戏整合
        cl;
        Game();
    }
    else if(FunctionChoice=="tool"){//工具整合
        cl;
        Tool();
    }
    else if(FunctionChoice=="sudo"){//索要管理员权限
        if(admin==false)AskAdmin();
        else cout<<"您已获得管理员权限,可直接在功能选择界面输入指令"<<endl;
        goto A;
    }
    else if(FunctionChoice=="clear"){//清屏
        cl;
    }
    else{
        if(admin==true)FunctionChoice="sudo "+FunctionChoice;
        ControlTerminal(FunctionChoice);
        goto A;
    }
    main();
    return 0;
}