#ifndef LOGINFORMFACTORY_H
#define LOGINFORMFACTORY_H
#include "loginform.h"
#include "mysqllite.h"

class LoginFormFactory
{
public:
    LoginFormFactory(); 
    static LoginForm* createLoginForm(int type, MySqlLite *sqltie,QWidget *parent = nullptr);
};

#endif // LOGINFORMFACTORY_H
