#include "util.hpp"
#include <memory>
#include <vector>

using func_t = function<void()>;

#define INIT(vc)                  \
    do                            \
    {                             \
        vc.push_back(printLog);   \
        vc.push_back(download);   \
        vc.push_back(executeSql); \
    } while (0)

#define EXEC_OTHER(cbs)            \
    do                             \
    {                              \
        for (const auto &vc : cbs) \
            vc();                  \
    } while (0)

int main()
{
    vector<func_t> cbs;
    INIT(cbs);

    SetNOBlock(0);
    char buffer[1024];
    while (true)
    {
        // printf(">> ");
        // fflush(stdout);
        ssize_t n = read(0, buffer, sizeof(buffer) - 1);
        if (n > 0)
        {
            buffer[n - 1] = 0;
            cout << "echo# " << buffer << endl;
        }
        else if (n == 0)
        {
            cout << "read end" << endl;
            break;
        }
        else
        {
            // 1. 当我不输入的时候，底层没有数据，算错误吗？不算错误，只不过以错误的形式返回了
            // 2. 我又如何区分，真的错了，还是底层没有数据？单纯返回值，无法区分！

            // cout<<"n : "<< n <<" errno : "<< strerror(errno) <<endl;
            // cout << "EAGAIN: " << EAGAIN << " EWOULDBLOCK: " << EWOULDBLOCK << endl;
            if (errno == EAGAIN || errno == EWOULDBLOCK)
            {
                //所以这里才是非阻塞返回时,底层数据没就绪,然后在干其他事情
                //这样才是正确写法
                cout << "我没有错,我只是没有数据..." << endl;
                EXEC_OTHER(cbs);
            }
            else if (errno == EINTR)
            {
                continue;
            }
            else
            {
                cout << "n : " << n << " errno: " << strerror(errno) << endl;
                break;
            }
        }
        sleep(2);
    }

    return 0;
}