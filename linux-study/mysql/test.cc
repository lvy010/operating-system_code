#include <iostream>
#include <mysql/mysql.h>
#include <string>
#include <unistd.h>

using namespace std;

const string host = "localhost";
const string user = "connector";
const string passwd = "123456";
const string db = "conn";
unsigned int port = 3306;

int main()
{
    // cout<<"mysql client Version: "<<mysql_get_client_info()<<endl;
    // return 0;

    // 1. 初始化mysql,构建mysql对象,得到mysql访问句柄
    MYSQL *my = mysql_init(nullptr);
    if (my == nullptr)
    {
        cerr << "init MySQL error" << endl;
        return 1;
    }

    // 2.连接数据库
    if (mysql_real_connect(my, host.c_str(), user.c_str(), passwd.c_str(), db.c_str(), port, nullptr, 0) == nullptr)
    {
        cerr << "connect MySQL error" << endl;
        return 2;
    }
    // cout<<"connect success"<<endl;

    // 设置编码格式
    mysql_set_character_set(my, "utf8");

    // string sql="update user set name='jim' where id=2";
    // string sql="insert into user (name,age,telphone) values ('peter',19,19187654321)";
    // string sql="insert into user (name,age,telphone) values ('李四',18,12187654321)";
    // string sql="delete from user where id=1";
    string sql = "select * from user";
    int n = mysql_query(my, sql.c_str());
    if (n == 0)
    {
        cout << sql << " success" << endl;
    }
    else
    {
        cout << sql << " error" << endl;
        return 3;
    }

    MYSQL_RES *res = mysql_store_result(my);
    if (res == nullptr)
    {
        cerr << "mysql_store_result error" << endl;
        return 4;
    }

    //下面都是和结果集有关的, MYSQL_RES
    my_ulonglong rows = mysql_num_rows(res);
    my_ulonglong cols = mysql_num_fields(res);

    cout << "行: " << rows << endl;
    cout << "列: " << cols << endl;

    //属性
    MYSQL_FIELD* fields=mysql_fetch_fields(res);

    for(int i=0;i<cols;++i)
    {
        cout<<fields[i].name<<"\t";
    }
    cout<<endl;

    //内容
    for(int i=0;i<rows;++i)
    {
        MYSQL_ROW line=mysql_fetch_row(res);
        for(int j=0;j<cols;++j)
        {
            cout<<line[j]<<"\t"; 
        }
        cout<<endl;

    }
    
    cout<<fields[0].db<<" "<<fields[0].table<<endl;

        // string sql;
        // while (true)
        // {
        //     cout << " mysql> ";
        //     if (!getline(cin, sql) || sql == "quit")
        //     {
        //         cout << "Bye" << endl;
        //         break;
        //     }

        //     int n = mysql_query(my, sql.c_str());
        //     if (n == 0)
        //     {
        //         cout << sql << " success" << endl;
        //     }
        //     else
        //     {
        //         cerr << sql << " error" << endl;
        //         return 3;
        //     }
        // }

        // sleep(10);

        //释放结果集
        mysql_free_result(res);

        // 3.用完之后close，释放资源
        mysql_close(my);

    return 0;
}