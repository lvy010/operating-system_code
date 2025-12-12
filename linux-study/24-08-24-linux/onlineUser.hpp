#pragma once

#include<iostream>
#include<string>
#include<unordered_map>

using namespace std;

class User
{
public:
    User(const string& ip,const uint16_t& port)
        :_ip(ip),_port(port)
    {}

    ~User()
    {}

    string& Getip()
    {
        return _ip;
    }

    uint16_t& Getport()
    {
        return _port;
    }

private:
    string _ip;
    uint16_t _port;
};


class onlineUser
{
public:
    onlineUser()
    {}

    void addUser(const string& ip,const uint16_t& port)
    {
        string id=ip+"#"+to_string(port);
        usp.insert(make_pair(id,User(ip,port)));
    }

    void eraseUser(const string& ip,const uint16_t& port)
    {
        string id=ip+"#"+to_string(port);
        usp.erase(id);
    }

    bool isOnline(const string& ip,const uint16_t& port)
    {
        string id=ip+"#"+to_string(port);
        return usp.find(id) != usp.end() ? true:false;
    }

    void boradcast(int sockfd,const string& ip,const uint16_t& port,const string& msg)
    {
        for(auto& us:usp)
        {
            struct sockaddr_in client;
            memset(&client,0,sizeof(client));
            client.sin_family=AF_INET;
            client.sin_addr.s_addr=inet_addr(us.second.Getip().c_str());
            client.sin_port=htons(us.second.Getport());
            string s=ip+"-"+to_string(port)+"# ";
            s+=msg;

            sendto(sockfd,s.c_str(),s.size(),0,(struct sockaddr*)&client,sizeof(client));
        }
    }

    ~onlineUser()
    {}

private:
    unordered_map<string,User> usp;

};