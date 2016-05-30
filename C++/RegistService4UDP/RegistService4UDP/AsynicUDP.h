#pragma once
#include <string>
#include <iostream>
using namespace std;
#include "Poco/Net/DatagramSocket.h"
#include "Poco/SharedPtr.h"
#include "Poco/Runnable.h"
#include "Poco/Thread.h"
#include "Poco/Threadpool.h"
#include "Poco/Mutex.h"
#include <functional>
#include <queue>



class UDPListener 
{
protected:
	virtual void onRecieve(char * ptr_data, int t_lenth) = 0;
};
void recieveData(Poco::Net::SocketAddress t_address, string t_data);

inline void recieveData(Poco::Net::SocketAddress t_address, string t_data)
{
	cout<< "recieve data : " << t_data<<endl;
	
}



class AsynicUDP : public Poco::Runnable
{

	class Packet
	{
	public:
		Poco::Net::SocketAddress address;
		string  data;
	};

public:
	AsynicUDP(UDPListener * ptr_obj)
	{
		_ptr_listener = ptr_obj;
		_udp_socket = new Poco::Net::DatagramSocket();
		_is_running = false;
		_is_setup = false;
	};

	~AsynicUDP()
	{
		stop();
	}

	AsynicUDP()
	{
		_ptr_listener = nullptr;
		_udp_socket = new Poco::Net::DatagramSocket();
	};
	
	bool setSocketOpt(string t_ip , int t_port)
	{
		try
		{
			_udp_socket->bind(Poco::Net::SocketAddress(t_ip, t_port));
			_udp_socket->setReceiveTimeout(Poco::Timespan(3,0));
		}
		catch (Poco::Exception ex)
		{
			cout << ex.message() << endl;
			return false;
		}
		return true;

	}

	bool start()
	{
		_is_running = true;
		_thread_pool.start(*this);
		Poco::Thread::sleep(10);  // ensure the first thread would setup 

		for (int i = 0; i < 10; i++)
		{
			_thread_pool.start(*this);  // setup the worker thread 
		}
		return true;
	}

	void stop()
	{
		_is_running = false;
		_thread_pool.joinAll();
	}

	bool sendTo(const Poco::Net::SocketAddress &t_address , const string & t_data)
	{
		if (!_is_running)
		{
			return false;
		}
		_udp_socket->sendTo(t_data.c_str(), t_data.size(), t_address);
	}

	virtual void onRecieve(Poco::Net::SocketAddress t_address, string t_data){}
	void setCallbck(function<void(Poco::Net::SocketAddress, string)> t_callback)
	{
		_delegate_callback = t_callback;
	}


protected:
	void run()
	{
		if (! _is_setup)
		{
			epoll();
		}
		/// for the worker thread 
		else
		{
			msgWorker();
		}
	}


	void epoll()
	{
		_is_setup = true;
		while (true){
			char i_buff[2048] = { 0 };
			Poco::Net::SocketAddress i_peeraddress;
			try
			{
				int recv_lenth = _udp_socket->receiveFrom(i_buff, sizeof(i_buff), i_peeraddress);
				if (recv_lenth == 0)
				{
					continue;
				}
				string i_data(i_buff, recv_lenth);


				Packet i_packet;
				i_packet.address = i_peeraddress;
				i_packet.data = i_data;
				{
					Poco::Mutex::ScopedLock i_lock(_lock);
					_data_queue.push(i_packet);
				}		
			}
			catch (Poco::Exception ex)
			{
				cout << ex.message() << endl;
			}
		}
	}

	void msgWorker()
	{
		while(_is_running)
		{
			Packet i_packet;
			{
				Poco::Mutex::ScopedLock i_lock(_lock);
				if (_data_queue.size() == 0)
				{
					Poco::Thread::sleep(10);
					continue;
				}
				i_packet = _data_queue.front();
				_data_queue.pop();
			}
			onRecieve(i_packet.address, i_packet.data);
			if (_delegate_callback){
				_delegate_callback(i_packet.address, i_packet.data);
			}
		}	
	}




private:
	UDPListener * _ptr_listener;
	Poco::SharedPtr<Poco::Net::DatagramSocket> _udp_socket;
	bool _is_running;
	function<void(Poco::Net::SocketAddress, string)> _delegate_callback;
	Poco::ThreadPool _thread_pool;
	Poco::Mutex  _lock;
	queue<Packet> _data_queue;
	bool          _is_setup;


public:




	static void main()
	{
		AsynicUDP i_this;

		if (!i_this.setSocketOpt("127.0.0.1", 8080))
		{
			cout << "adsress already in use " << endl;
			return;
		}
		i_this.setCallbck(recieveData);
		i_this.start();
		system("pause");
		//string i_data = "1234567890";
		//for (int i = 0; i < 10; i++)
		//{
		//	i_data += i_data;
		//}

		//i_this.sendTo(Poco::Net::SocketAddress("127.0.0.1", 8082), i_data);
		//system("pause");
		//
	}



};

