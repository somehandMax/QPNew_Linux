#include <iostream>
#include <unistd.h> 
using namespace std;
int main() {
    while (true) {
        cout << endl;
        cout << "1.必应" << endl;
        cout << "2.为百度" << endl;
        cout << "3.为Google" << endl;
        cout << "4.为B站" << endl;
        cout << "5.为文心一言" << endl;
        cout << "6.Ping测试" << endl;
        cout << "7.为洛谷" << endl;
        cout << "8.为一本通" << endl;
        cout << "9.来追梦OJ" << endl;
        cout << "a.为leetcode" << endl;
        cout << "b.为MSDN(旧)" << endl;
        cout << "c.为MSDN(新)" << endl;
        cout << "d.为网易云" << endl;
        cout << "e.为知乎网站推荐" << endl;
        cout << "f.为孟昆工具网" << endl;
        cout << "g.Edge扩展：VPN/代理" << endl;
        cout << "h.免费New Bing AI" << endl;
        cout << "退出为A" << endl;
        char choicewebsite;
        cin>>choicewebsite;
        switch (choicewebsite) {
            case '1':system("xdg-open https://www.bing.com");break;
            case '2':system("xdg-open https://www.baidu.com");break;
            case '3':system("xdg-open https://www.google.com");break;
            case '4':system("xdg-open https://www.bilibili.com");break;
            case '5':system("xdg-open https://yiyan.baidu.com/");break;
            case '6':
                system("ping www.baidu.com");
                cout << "输入1退出" << endl;
                while (true) break;
                break;
            case '7':system("xdg-open https://www.luogu.com.cn");break;
            case '8':system("xdg-open http://ybt.ssoier.cn:8088/");break;
            case '9':system("xdg-open https://gonoi.com.cn/problem?page=1&k=&s=&tagIds=");break;
            case 'a':system("xdg-open https://leetcode.cn/");break;
            case 'b':system("xdg-open https://msdn.itellyou.cn/");break;
            case 'c':system("xdg-open https://next.itellyou.cn/Identity/Account/Login?ReturnUrl=%2FOriginal%2FIndex");break;
            case 'd':system("xdg-open https://music.163.com");break;
            case 'e':system("xdg-open https://zhuanlan.zhihu.com/p/361873198");break; 
            case 'f':system("xdg-open https://lab.ur1.fun/");break;           
            case 'g':system("xdg-open https://microsoftedge.microsoft.com/addons/detail/ilink%E7%BD%91%E7%BB%9C%E5%8A%A0%E9%80%9F%E5%99%A8/ekkkldemlhnacnfegdihallelglbnhnl");break;
            case 'h':system("xdg-open https://blog.laogou717.com/timeline/");break;
            case 'A':return 0;
            
        }
        usleep(500000);
        system("clear");
    }
    
    return 0;
}
