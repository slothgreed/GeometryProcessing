#ifndef THE_APP_H
#define	THE_APP_H
#include "TheApp.h"
class TheApp
{
public:
	TheApp() {};
	~TheApp() {};
	virtual void Initialize() {};
	virtual void Execute() {};
	virtual void Finalize() {};
private:

};

#endif // THE_APP_H