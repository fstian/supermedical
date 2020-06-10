package com.yt.lib;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.ServerSocket;
import java.net.Socket;
import java.net.SocketException;

import io.netty.bootstrap.ServerBootstrap;
import io.netty.channel.ChannelFuture;
import io.netty.channel.ChannelInitializer;
import io.netty.channel.ChannelOption;
import io.netty.channel.ChannelPipeline;
import io.netty.channel.EventLoopGroup;
import io.netty.channel.nio.NioEventLoopGroup;
import io.netty.channel.socket.SocketChannel;
import io.netty.channel.socket.nio.NioServerSocketChannel;
import io.netty.handler.codec.LineBasedFrameDecoder;
import io.netty.handler.codec.string.StringDecoder;
import io.netty.handler.timeout.IdleStateHandler;

public class MyClass {


    static  int port=11676;

    public static void main(String[] args) {


        netty();


    }


    private  static  void netty() {

        try {
            //1:第一个线程组是用于接收Client连接的
            EventLoopGroup bossGroup = new NioEventLoopGroup(); // boss

            //2:第二个线程组是用于实际的业务处理操作的
            EventLoopGroup workerGroup = new NioEventLoopGroup(); // worker

            //3:创建一个启动NIO服务的辅助启动类ServerBootstrap 就是对我们的Server进行一系列的配置
            ServerBootstrap serverBootstrap = new ServerBootstrap();
            serverBootstrap.group(bossGroup, workerGroup)
                    .channel(NioServerSocketChannel.class)
                    .childHandler(new ChannelInitializer<SocketChannel>() {
                        @Override
                        protected void initChannel(SocketChannel socketChannel) throws Exception {

                            ChannelPipeline pipeline = socketChannel.pipeline();
                            pipeline.addLast(new LineBasedFrameDecoder(10240));
                            pipeline.addLast(new StringDecoder());
                            pipeline.addLast(new ServerHandler());


                        }
                    })
                    .option(ChannelOption.SO_BACKLOG, 128)
                    .childOption(ChannelOption.SO_KEEPALIVE, true);
            ChannelFuture cf2 = serverBootstrap.bind(port).sync(); // (7)
        } catch (InterruptedException e) {
            e.printStackTrace();
        }finally {

        }

    }






    private static void test() {

        int count = 0;
        try {
            ServerSocket serverSocket = new ServerSocket(17878);

            while (true) {
                count++;
                Socket accept = serverSocket.accept();
                ReceiveThread receiveThread = new ReceiveThread(accept);
                receiveThread.setName("threadName" + count);
                receiveThread.start();
                System.out.println("count" + count);

            }

        } catch (IOException e) {
            System.out.println("err" + e.getMessage());

            e.printStackTrace();
        }
    }


    static class ReceiveThread extends Thread {

        private Socket mSocket;

        public ReceiveThread(Socket socket) {
            mSocket = socket;
        }

        public void sendMsg(String text) {

            try {
                mSocket.getOutputStream().write((text + "\n").getBytes());
            } catch (IOException e) {
                e.printStackTrace();
            }

        }

        @Override
        public void run() {
            try {
                InputStream is = mSocket.getInputStream();
                BufferedReader reader = new BufferedReader(new InputStreamReader(is));
                byte[] bytes = new byte[1024];
                int len = 0;
                String text;

//                while ((len = is.read(bytes)) > 0) {
                while (true) {
                    System.out.println(getName());
                    sendMsg(getName());
                    Thread.sleep(5*1000);
                }


            } catch (IOException e) {
                e.printStackTrace();
            } catch (InterruptedException e) {
                e.printStackTrace();
            }


        }
    }

}
