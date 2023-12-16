#pragma once

namespace Snail
{

template <class T>
class Singleton
{
public:
	Singleton(Singleton&) = delete;
	void operator=(Singleton&) = delete;

    static T& GetInstance()
    {
		static T Instance;
        return Instance;
    }

protected:
	Singleton(){}
	virtual ~Singleton(){}
};

}