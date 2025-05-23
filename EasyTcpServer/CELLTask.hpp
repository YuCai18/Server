#ifndef _CELL_TASK_H_
#include<thread>
#include<mutex>
#include<list>
#include <functional>

//任务类型-基类
class CellTask
{
public:
	CellTask()
	{

	}

	//虚析构 虚指针 虚表
	virtual ~CellTask()
	{

	}
	//执行任务
	virtual void doTask()
	{
		

	}
private:


};
 
//执行任务的服务类型
class CellTaskServer
{
private:
	//任务数据
	std::list<CellTask*> _tasks;
	//任务数据缓冲区
	std::list<CellTask*> _tasksBuf;
	//改变数据缓冲区的时候需要加锁
	std::mutex _mutex;

public:

	CellTaskServer()
	{

	}
	~CellTaskServer()
	{

	}
	//添加任务
	void addTask(CellTask* task)
	{
		std::lock_guard<std::mutex> lock(_mutex);

		_tasksBuf.push_back(task);
	}
	//启动服务
	void Start()
	{
		//创建线程并且启动线程
		std::thread t(std::mem_fn(&CellTaskServer::OnRun),this);
		t.detach();
	}
protected:
	//工作函数
	void OnRun()
	{
		while (true)
		{
			//从缓冲区里取出数据
			if (!_tasksBuf.empty())
			{
				std::lock_guard<std::mutex> lock(_mutex);
				for (auto pTask : _tasksBuf)
				{
					_tasks.push_back(pTask);

				}
				_tasksBuf.clear();
			}
			
			//如果没有任务
			if (_tasks.empty())
			{
				std::chrono::milliseconds t(1);
				std::this_thread::sleep_for(t);
				continue;

			}
			//处理任务
			for (auto pTask : _tasks)
			{
				pTask->doTask();
				delete pTask; 
			}
			//清空任务
			_tasks.clear();
		}
		

	}
};

#endif // !_CELL_TASK_H_

