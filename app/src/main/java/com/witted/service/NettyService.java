package com.witted.service;

import android.app.Service;
import android.content.Intent;
import android.os.IBinder;
import android.util.Log;

import com.witted.service.remote.GeneralCallback;

import java.io.UnsupportedEncodingException;
import java.util.concurrent.TimeUnit;

import io.netty.bootstrap.Bootstrap;
import io.netty.buffer.ByteBuf;
import io.netty.buffer.Unpooled;
import io.netty.channel.ChannelFuture;
import io.netty.channel.ChannelInitializer;
import io.netty.channel.ChannelOption;
import io.netty.channel.ChannelPipeline;
import io.netty.channel.nio.NioEventLoopGroup;
import io.netty.channel.socket.SocketChannel;
import io.netty.channel.socket.nio.NioSocketChannel;
import io.netty.handler.codec.LineBasedFrameDecoder;
import io.netty.handler.codec.string.StringDecoder;
import io.netty.handler.timeout.IdleStateHandler;

public class NettyService extends Service {
    private static final String TAG = NettyService.class.getName();
    private ChannelFuture mSync;

    @Override
    public IBinder onBind(Intent intent) {
        return null;
    }

    @Override
    public void onCreate() {
        super.onCreate();

        new Thread(new Runnable() {
            @Override
            public void run() {
//                nettyClient("172.16.1.172", 11676);
                nettyClient("172.16.2.209", 9999);
            }
        }).start();

//        nettyClient("172.16.1.20", 11676);

    }


    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {

        Log.i(TAG, "onStartCommand: ");


        return super.onStartCommand(intent, flags, startId);
    }


    private void nettyClient(String host, int port) {
        NioEventLoopGroup workGroup=null;
        try {
             workGroup = new NioEventLoopGroup();
            Bootstrap bootstrap = new Bootstrap();
            bootstrap.group(workGroup)
                    .channel(NioSocketChannel.class)
                    .option(ChannelOption.SO_KEEPALIVE, true)
                    .handler(new ChannelInitializer<SocketChannel>() {
                        @Override
                        protected void initChannel(SocketChannel socketChannel) {
                            ChannelPipeline pipeline = socketChannel.pipeline();
                            pipeline.addLast(new LineBasedFrameDecoder(10240));
                            pipeline.addLast("ping", new IdleStateHandler(0, 30, 0, TimeUnit.SECONDS));
                            pipeline.addLast(new StringDecoder());
//                            pipeline.addLast(new HeartBeatHandler());
                            ClientHandler clientHandler = new ClientHandler();
                            clientHandler.setCallBack(mGeneralCallback);
                            pipeline.addLast(clientHandler);
                            pipeline.addLast(new HeartBeatHandler());
                        }
                    });
            mSync = bootstrap.connect(host, port).sync();
            mSync.channel().closeFuture().sync();

        } catch (Exception e) {
            Log.e(TAG, "nettyClient: " + e.getMessage());
            e.printStackTrace();
        }finally {
            workGroup.shutdownGracefully();
        }

    }


    GeneralCallback mGeneralCallback = new GeneralCallback() {
        @Override
        public void onSuccess() {
            Log.i(TAG, "onSuccess: " + Thread.currentThread().getName());


        }

        @Override
        public void onFail(String err) {
            Log.i(TAG, "onFail: " + err + Thread.currentThread().getName());

        }
    };

    private ByteBuf sendMsg(String msg)  {

        try {
            byte[] bytes = (msg+System.getProperty("line.separator")).getBytes("UTF-8");
            ByteBuf buffer = Unpooled.buffer(bytes.length);
            buffer.writeBytes(bytes);
            return buffer;
        } catch (UnsupportedEncodingException e) {
            Log.e(TAG, "sendMsg: "+e.getMessage());
            e.printStackTrace();
        }

        return null;
    }

}
