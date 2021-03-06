#pragma once

#include <memory>
#include <string>
#include <atomic>

#include "base/types.h"
#include "base/NonCopyable.h"
#include "base/Timestamp.h"
#include "base/any.h"
#include "base/StringPiece.h"

#include "CallBack.h"
#include "InetAddress.h"
#include "Buffer.h"

namespace MuduoPlus
{
    class Channel;
    class EventLoop;

    class TcpConnection : NonCopyable,
        public std::enable_shared_from_this<TcpConnection>
    {
    public:
        /// Constructs a TcpConnection with a connected sockfd
        ///
        /// User should not create this object.
        TcpConnection(EventLoop* loop,
                      const std::string& name,
                      int sockfd,
                      const InetAddress& localAddr,
                      const InetAddress& peerAddr);
        ~TcpConnection();

        EventLoop* getLoop() const
        {
            return loop_;
        }
        const std::string& name() const
        {
            return name_;
        }
        const InetAddress& localAddress() const
        {
            return localAddr_;
        }
        const InetAddress& peerAddress() const
        {
            return peerAddr_;
        }
        bool connected() const
        {
            return state_ == kConnected;
        }
        bool disconnected() const
        {
            return state_ == kDisconnected;
        }
        // return true if success.
        /*bool getTcpInfo(struct tcp_info*) const;
        std::string getTcpInfoString() const;*/

        // void send(string&& message); // C++11
        void send(const void* data, int len);
        void send(const StringPiece& message);
        //void send(Buffer* message);  // this one will swap data
        void gracefulClose(); // NOT thread safe, no simultaneous calling
        // void shutdownAndForceCloseAfter(double seconds); // NOT thread safe, no simultaneous calling
        void forceClose();
        void forceCloseWithDelay(double seconds);
        void setTcpNoDelay(bool on);
        void startRead();
        void stopRead();
        bool isReading() const
        {
            return reading_;
        }; // NOT thread safe, may race with start/stopReadInLoop

        Any&    getContext()
        {
            return context_;
        }
        void    setContext(Any val)
        {
            context_ = val;
        }

        void setConnectionCallback(const ConnectionCallback& cb)
        {
            connectionCallback_ = cb;
        }

        void setMessageCallback(const MessageCallback& cb)
        {
            messageCallback_ = cb;
        }

        void setWriteCompleteCallback(const WriteCompleteCallback& cb)
        {
            writeCompleteCallback_ = cb;
        }

        void setHighWaterMarkCallback(const HighWaterMarkCallback& cb, size_t highWaterMark)
        {
            highWaterMarkCallback_ = cb;
            highWaterMark_ = highWaterMark;
        }

        /// Advanced interface
        Buffer* inputBuffer()
        {
            return &inputBuffer_;
        }

        Buffer* outputBuffer()
        {
            return &outputBuffer_;
        }

        /// Internal use only.
        void setCloseCallback(const CloseCallback& cb)
        {
            closeCallback_ = cb;
        }

        // called when TcpServer accepts a new connection
        void connectEstablished();   // should be called only once
        // called when TcpServer has removed me from its map
        void connectDestroyed();  // should be called only once

    private:
        enum StateE { kDisconnected, kConnecting, kConnected, kDisconnecting };
        void handleRead(Timestamp receiveTime);
        void handleWrite();
        void handleError();
        void handleEnd();
        void sendInLoop(const StringPiece& message);
        void sendInLoop(const void* data, int len);
        void shutdownInLoop();
        // void shutdownAndForceCloseInLoop(double seconds);
        void forceCloseInLoop();
        void releaseConnection();
        void setState(StateE s)
        {
            state_ = s;
        }
        const char* stateToString() const;
        void startReadInLoop();
        void stopReadInLoop();

        EventLoop* loop_;
        const std::string name_;
        std::atomic<StateE> state_;
        // we don't expose those classes to client.
        int fd_;
        bool sockErrorOccurred_;
        bool userClosed_;
        std::shared_ptr<Channel> channel_;
        const InetAddress localAddr_;
        const InetAddress peerAddr_;
        ConnectionCallback connectionCallback_;
        MessageCallback messageCallback_;
        WriteCompleteCallback writeCompleteCallback_;
        HighWaterMarkCallback highWaterMarkCallback_;
        CloseCallback closeCallback_;
        size_t highWaterMark_;
        Buffer inputBuffer_;
        Buffer outputBuffer_; // FIXME: use list<Buffer> as output buffer.
        bool    reading_;
        Any     context_;
    };

    typedef std::shared_ptr<TcpConnection> TcpConnectionPtr;
}