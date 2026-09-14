#include "loginformfactory.h"

LoginFormFactory::LoginFormFactory()
{

}

LoginForm *LoginFormFactory:: createLoginForm(int type, MySqlLite *sqltie,QWidget* parent ) {
    return new LoginForm(type, sqltie, parent);
}
