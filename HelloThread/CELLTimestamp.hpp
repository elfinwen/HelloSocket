#ifndef _CELLTimestamp_hpp_
#define _CELLTimestamp_hpp_

//#include <windows.h>

//在C++11中，<chrono>是标准模板库中与时间有关的头文件。该头文件中所有函数与类模板均定义在
//std::chrono命名空间中。
#include<chrono>
using namespace std::chrono;

class CELLTimestamp
{
public:
    CELLTimestamp()
    {
        //QueryPerformanceFrequency(&_frequency);
        //QueryPerformanceCounter(&_startCount);
		update();
    }
    ~CELLTimestamp()
    {}

    void    update()
    {
        //QueryPerformanceCounter(&_startCount);
		_begin = high_resolution_clock::now();
    }
    /**
    *   获取当前秒
    */
    double getElapsedSecond()
    {
        return  getElapsedTimeInMicroSec() * 0.000001;
    }
    /**
    *   获取毫秒
    */
    double getElapsedTimeInMilliSec()
    {
        return this->getElapsedTimeInMicroSec() * 0.001;
    }
    /**
    *   获取微妙
    */
    long long getElapsedTimeInMicroSec()
    {
		/*
        LARGE_INTEGER endCount;
        QueryPerformanceCounter(&endCount);

        double  startTimeInMicroSec =   _startCount.QuadPart * (1000000.0 / _frequency.QuadPart);
        double  endTimeInMicroSec   =   endCount.QuadPart * (1000000.0 / _frequency.QuadPart);

        return  endTimeInMicroSec - startTimeInMicroSec;
		*/
		//微秒microseconds(毫秒milliseconds)
		return duration_cast<microseconds>(high_resolution_clock::now() - _begin).count();
    }
protected:
    //LARGE_INTEGER   _frequency;
    //LARGE_INTEGER   _startCount;

	//high_resolution_clock 高分辨率时钟，可以精确到微秒，甚至纳秒，是很精准的计时器。它是通
	//过CPU的时间片来计算的，是很底层的代码实现
	time_point<high_resolution_clock> _begin;
};

#endif // !_CELLTimestamp_hpp_