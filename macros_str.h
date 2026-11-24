#ifndef MACROS_STR_H
#define MACROS_STR_H

#define STR(__arg__) QString("%1").arg(__arg__)

#define STR1(__txt__, __arg1__) QString(__txt__ ).arg(__arg1__)
#define STR2(__txt__, __arg1__, __arg2__) QString(__txt__ ).arg(__arg1__).arg(__arg2__)
#define STR3(__txt__, __arg1__, __arg2__, __arg3__) QString(__txt__ ).arg(__arg1__).arg(__arg2__).arg(__arg3__)

#endif // MACROS_STR_H
