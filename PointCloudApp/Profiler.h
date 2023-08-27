#ifndef PROFILE_H
#define PROFILE_H
#include <windows.h>

class Profile
{
public:
	Profile();
	~Profile();

	void Start();
	void Stop();
	void Output();
private:
	LARGE_INTEGER m_freq;
	LARGE_INTEGER m_start;
	LARGE_INTEGER m_end;

};

#endif PROFILE_H