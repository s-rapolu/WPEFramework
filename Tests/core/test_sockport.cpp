#include "../IPTestAdministrator.h"

#include <gtest/gtest.h>
#include <core/core.h>
#include <core/SocketPort.h>

namespace WPEFramework {
  namespace Tests {

   namespace StreamSockTest {
      const TCHAR* g_connector = "/tmp/streamtext0";
      bool g_done = false;
    } // StreamSockTest

    namespace SockTest {
      const string message = "hello UDP network";
      const string localhost = "127.0.0.1";
      const uint16_t portNumber = 7099;
      const uint16_t bufferSize = 1024;
      const string mcastAddress = "239.4.4.4";
      const uint16_t mcastPort = 8888;
    } // SockTest

    class UDPServer : public Core::StreamType<Core::SocketDatagram> {
      private:
        UDPServer() = delete;
        UDPServer(const UDPServer&) = delete;
        UDPServer& operator=(const UDPServer&) = delete;

        typedef Core::StreamType<Core::SocketDatagram> BaseClass;

     public:
        UDPServer(const WPEFramework::Core::NodeId& localNode, const WPEFramework::Core::NodeId& remoteNode)
          : BaseClass(false, localNode.AnyInterface(), remoteNode, SockTest::bufferSize, SockTest::bufferSize)
          , _dataPending(false, false)
        {
        }
        virtual ~UDPServer()
        {
        }

        int Wait() const
        {
          return _dataPending.Lock();
        }
        void Retrieve(string& text)
        {
          text = _dataReceived;
          _dataReceived.clear();
        }
        void SendSubmit(const string& text)
        {
          _dataReceived = text;
          Trigger();
        }
        virtual uint16_t SendData(uint8_t* dataFrame, const uint16_t maxSendSize)
        {
          uint16_t result = 0;
          if (BaseClass::LocalNode().IsMulticast() == true) {
            result = (_dataReceived.length() > maxSendSize ? maxSendSize : _dataReceived.length());
            ::memcpy(dataFrame, _dataReceived.c_str(), result);
            //_dataReceived.clear();
            _dataPending.Unlock();
          }

          return result;
        }
        virtual uint16_t ReceiveData(uint8_t* dataFrame, const uint16_t receivedSize)
        {
          _dataReceived = reinterpret_cast<char*>(dataFrame);
          _dataPending.Unlock();
          return _dataReceived.size();
        }
        virtual void StateChange()
        {
        }
      private:
        string _dataReceived;
        mutable WPEFramework::Core::Event _dataPending;
    };

    class UDPClient : public Core::StreamType<Core::SocketDatagram> {
      private:
        UDPClient() = delete;
        UDPClient(const UDPClient&) = delete;
        UDPClient& operator=(const UDPClient&) = delete;

        typedef Core::StreamType<Core::SocketDatagram> BaseClass;

     public:
        UDPClient(const WPEFramework::Core::NodeId& localNode, const WPEFramework::Core::NodeId& remoteNode)
          : BaseClass(false, localNode.AnyInterface(), remoteNode, SockTest::bufferSize, SockTest::bufferSize)
          , _dataPending(false, false)
        {
        }
        virtual ~UDPClient()
        {
        }

        int Wait()
        {
          //Trigger();
          return _dataPending.Lock();
        }
        void Retrieve(string& text)
        {
          text = _dataReceived;
          _dataReceived.clear();
        }
        void SendSubmit(const string& text)
        {
          _dataReceived = text;
          Trigger();
        }
        virtual uint16_t SendData(uint8_t* dataFrame, const uint16_t maxSendSize)
        {
          uint16_t result = (_dataReceived.length() > maxSendSize ? maxSendSize : _dataReceived.length());
          ::memcpy(dataFrame, _dataReceived.c_str(), result);
          _dataReceived.clear();
          _dataPending.Unlock();
          return result;
        }
        virtual uint16_t ReceiveData(uint8_t* dataFrame, const uint16_t receivedSize)
        {
          uint16_t result = 0;
          if (BaseClass::RemoteNode().IsMulticast() == true) {
            _dataReceived = reinterpret_cast<char*>(dataFrame);
            result = (_dataReceived.length() > receivedSize ? receivedSize : _dataReceived.length());
            _dataPending.Unlock();
          }
          return result;
        }

        virtual void StateChange()
        {
        }
      private:
        string _dataReceived;
        mutable WPEFramework::Core::Event _dataPending;
    };

    class StreamTextConnector : public Core::StreamTextType<Core::SocketStream, Core::TerminatorNull> {
      private:
        typedef Core::StreamTextType<Core::SocketStream, Core::TerminatorNull> BaseClass;

        StreamTextConnector();
        StreamTextConnector(const StreamTextConnector& copy);
        StreamTextConnector& operator=(const StreamTextConnector&);

      public:
        StreamTextConnector(const WPEFramework::Core::NodeId& remoteNode)
          : BaseClass(false, remoteNode.AnyInterface(), remoteNode, SockTest::bufferSize, SockTest::bufferSize)
          , _serverSocket(false)
          , _dataPending(false, false)
          {
          }

          StreamTextConnector(const SOCKET& connector, const Core::NodeId& remoteId, Core::SocketServerType<StreamTextConnector>*)
            : BaseClass(false, connector, remoteId, SockTest::bufferSize, SockTest::bufferSize)
            , _serverSocket(true)
            , _dataPending(false, false)
            {
            }

            virtual ~StreamTextConnector()
            {
            }

      public:
        virtual void Received(string& text)
        {
          if (_serverSocket)
            Submit(text);
          else {
            _dataReceived = text;
            _dataPending.Unlock();
          }
        }
        int Wait() const
        {
          return _dataPending.Lock();
        }
        void Retrieve(string& text)
        {
          text = _dataReceived;
          _dataReceived.clear();
        }
        virtual void Send(const string& text)
        {
        }
        virtual void StateChange()
        {
          if (IsOpen()) {
            if (_serverSocket)
              StreamSockTest::g_done = true;
          }
        }

      private:
        bool _serverSocket;
        string _dataReceived;
        mutable WPEFramework::Core::Event _dataPending;
    };

   TEST(DISABLED_Core_SocketPort, test_SockDatagramUnicast)
   {
      IPTestAdministrator::OtherSideMain otherSide = [](IPTestAdministrator& testAdmin) {
        UDPServer udpSocketServerUnicast(Core::NodeId(_T(SockTest::localhost.c_str()), SockTest::portNumber, Core::NodeId::TYPE_IPV4), Core::NodeId());
        udpSocketServerUnicast.Open(Core::infinite);
        testAdmin.Sync("setup server");
        testAdmin.Sync("server open");
        udpSocketServerUnicast.Wait();
        string messageReceived;
        //Receive message from Server side
        udpSocketServerUnicast.Retrieve(messageReceived);
	//TODO-2: Test with different send and receive buffer sizes.
        EXPECT_STREQ(SockTest::message.c_str(), messageReceived.c_str());
        testAdmin.Sync("close socket");
        udpSocketServerUnicast.Close(Core::infinite);
        testAdmin.Sync("client done");
     };

     IPTestAdministrator testAdmin(otherSide);
     testAdmin.Sync("setup server");

     {
        UDPClient udpSocketClientUnicast(Core::NodeId(_T(SockTest::localhost.c_str()), (SockTest::portNumber-1), Core::NodeId::TYPE_IPV4),
                                        Core::NodeId(_T(SockTest::localhost.c_str()), SockTest::portNumber, Core::NodeId::TYPE_IPV4));
        udpSocketClientUnicast.Open(Core::infinite);
        testAdmin.Sync("server open");
        udpSocketClientUnicast.SendSubmit(SockTest::message);
        udpSocketClientUnicast.Wait();
        testAdmin.Sync("close socket");
        udpSocketClientUnicast.Close(Core::infinite);
        testAdmin.Sync("client done");
      }

      Core::Singleton::Dispose();
   }

   TEST(Core_SocketPort, test_SockDatagramMulticast)
   {
      IPTestAdministrator::OtherSideMain otherSide = [](IPTestAdministrator& testAdmin) {
        UDPServer udpSocketServerMulticast(Core::NodeId(_T("0.0.0.0"), 8989, Core::NodeId::TYPE_IPV4),
                        Core::NodeId(_T(SockTest::mcastAddress.c_str()), SockTest::mcastPort, Core::NodeId::TYPE_IPV4));
        udpSocketServerMulticast.Open(Core::infinite);
        testAdmin.Sync("setup server");
        testAdmin.Sync("server open");
        udpSocketServerMulticast.SendSubmit(SockTest::message);
        udpSocketServerMulticast.Wait();
        testAdmin.Sync("close socket");
        udpSocketServerMulticast.Close(Core::infinite);
        testAdmin.Sync("client done");
      };

      IPTestAdministrator testAdmin(otherSide);
      testAdmin.Sync("setup server");

      {
        UDPClient udpSocketClientMulticast(Core::NodeId(_T(SockTest::mcastAddress.c_str()), SockTest::mcastPort, Core::NodeId::TYPE_IPV4),
                                        Core::NodeId());
        udpSocketClientMulticast.Open(Core::infinite);
	// TODO-1: Later join with specific ethernet interface or source
	// TODO-3: Test the problem of binding to a port already in use
	// TODO-4: Test the timeouts on the sockets
	// TODO-5: Test the scenario of closing down server socket when client is half way through send/recv.
	// TODO-6: Test the blocking and non-blocking mode of socket operations
        udpSocketClientMulticast.Join(Core::NodeId(_T(SockTest::mcastAddress.c_str()))); 
        testAdmin.Sync("server open");
        udpSocketClientMulticast.Wait();
        string messageReceived;
        udpSocketClientMulticast.Retrieve(messageReceived);
        EXPECT_STREQ(SockTest::message.c_str(), messageReceived.c_str());
        testAdmin.Sync("close socket");
        udpSocketClientMulticast.Leave(Core::NodeId(_T(SockTest::mcastAddress.c_str())));
        udpSocketClientMulticast.Close(Core::infinite);
        testAdmin.Sync("client done");
      }

      Core::Singleton::Dispose();
    }

    TEST(DISABLED_Core_SocketPort, test_SockStream)
    {
      IPTestAdministrator::OtherSideMain otherSide = [](IPTestAdministrator& testAdmin) {
        Core::SocketServerType<StreamTextConnector> textSocketServer(Core::NodeId(StreamSockTest::g_connector));

        textSocketServer.Open(Core::infinite);
        testAdmin.Sync("setup server");
        while (!StreamSockTest::g_done);
        testAdmin.Sync("server open");
        testAdmin.Sync("client done");
      };

      IPTestAdministrator testAdmin(otherSide);
      testAdmin.Sync("setup server");

      {
        StreamTextConnector textSocketClient(Core::NodeId(StreamSockTest::g_connector));
        textSocketClient.Open(Core::infinite);
        testAdmin.Sync("server open");
        string TCPMessage = "hello TCP network";
        textSocketClient.Submit(TCPMessage);
        textSocketClient.Wait();
        string received;
        textSocketClient.Retrieve(received);
        EXPECT_STREQ(TCPMessage.c_str(), received.c_str());
        textSocketClient.Close(Core::infinite);
        testAdmin.Sync("client done");
      }

      Core::Singleton::Dispose();
    }
  } // Tests
} // WPEFramework
