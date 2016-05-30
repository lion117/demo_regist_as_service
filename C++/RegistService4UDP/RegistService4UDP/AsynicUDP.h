#pragma once
#include <string>
#include <iostream>
using namespace std;
#include "Poco/Net/DatagramSocket.h"
#include "Poco/SharedPtr.h"
#include "Poco/Runnable.h"
#include "Poco/Thread.h"
#include "Poco/Net/NetException.h"

#include <functional>


void onRecieve11(Poco::Net::SocketAddress t_address, string t_data);

inline void onRecieve11(Poco::Net::SocketAddress t_address,  string t_data)
{
	string i_echo = "echo:  " + t_data;
	cout << i_echo << endl;
	//_udp_socket->sendTo(i_echo.c_str(), i_echo.size(), t_address);
}


class UDPListener 
{
protected:
	virtual void onRecieve(char * ptr_data, int t_lenth) = 0;
};




class AsynicUDP : public Poco::Runnable
{
public:
	AsynicUDP(UDPListener * ptr_obj)
	{
		_ptr_listener = ptr_obj;
		_udp_socket = new Poco::Net::DatagramSocket();
		_is_running = false;
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
		_thread.start(*this);
		return false;
	}

	void stop()
	{
		_is_running = false;
		if (_thread.isRunning())
		{
			_thread.join();
		}
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


protected:
	void run()
	{
		while (_is_running)
		{
			char i_buff[2048] = {0};
			Poco::Net::SocketAddress i_peeraddress;
			try
			{
				int recv_lenth = _udp_socket->receiveFrom(i_buff, sizeof(i_buff), i_peeraddress);
				if (recv_lenth == 0)
				{
					continue;
				}
				string i_data(i_buff, recv_lenth);
				onRecieve(i_peeraddress, i_data);
				if(_delegate_callback){
					_delegate_callback(i_peeraddress, i_data);
				}


			}
			catch (Poco::Exception ex)
			{
				cout<< ex.message()<<endl;
			}
			
		}
	}




private:
	UDPListener * _ptr_listener;
	Poco::SharedPtr<Poco::Net::DatagramSocket> _udp_socket;
	Poco::Thread  _thread;
	bool _is_running;
	function<void(Poco::Net::SocketAddress, string)> _delegate_callback;


public:

	void setCallbck(function<void(Poco::Net::SocketAddress, string)> t_callback)
	{
		_delegate_callback = t_callback;
	}

	static void main()
	{
		AsynicUDP i_this;

		if (!i_this.setSocketOpt("127.0.0.1", 8080))
		{
			cout << "adsress already in use " << endl;
			return;
		}
		i_this.setCallbck(onRecieve11);
		i_this.start();
		system("pause");
		string i_data = "1234567890";
		for (int i = 0; i < 10; i++)
		{
			i_data += i_data;
		}

		i_this.sendTo(Poco::Net::SocketAddress("127.0.0.1", 8082), i_data);
		system("pause");
		
	}



};

