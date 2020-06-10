package com.witted.service;

import android.util.Log;

import com.alibaba.fastjson.JSON;
import com.witted.service.model.BaseRequest;
import com.witted.service.model.HeartBeat;
import com.witted.service.model.Register;
import com.witted.service.remote.GeneralCallback;

import java.io.UnsupportedEncodingException;
import java.util.ArrayList;
import java.util.concurrent.TimeUnit;

import io.netty.buffer.ByteBuf;
import io.netty.buffer.Unpooled;
import io.netty.channel.ChannelHandlerContext;
import io.netty.channel.ChannelInboundHandlerAdapter;
import io.netty.handler.codec.LineBasedFrameDecoder;
import io.netty.handler.timeout.IdleStateHandler;
import io.netty.util.CharsetUtil;
import io.netty.util.ReferenceCountUtil;

public class ClientHandler extends ChannelInboundHandlerAdapter {

    private static final String TAG = ClientHandler.class.getName();

    String test= "Completely destroy information stored without your knowledge or approval: Internet history, " +
            "Web pages and pictures from sites visited on the Internet, unwanted cookies, chatroom conversations, deleted e-mail " +
            "messages, temporary files, the Windows swap file, the Recycle Bin, previously deleted files, valuable corporate trade secrets," +
            " Business plans, personal files, photos or confidential letters, etc.East-Tec Eraser 2005 offers full support for popular browsers" +
            " (Internet Explorer, Netscape Navigator, America Online, MSN Explorer, Opera), for Peer2Peer applications (Kazaa, Kazaa Lite, iMesh," +
            " Napster, Morpheus, Direct Connect, Limewire, Shareaza, etc.), and for other popular programs such as Windows Media Player, " +
            "RealPlayer, Yahoo Messenger, ICQ, etc. Eraser has an intuitive interface and wizards that guide you through all the necessary " +
            "steps needed to protect your privacy and sensitive information.Other features include support for custom privacy needs," +
            " user-defined erasure methods, command-line parameters, integration with Windows Explorer, and password protection.Completely" +
            " destroy information stored without your knowledge or approval: Internet history,"+
            "steps needed to protect your privacy and sensitive information.Other features include support for custom privacy needs," +
            "steps needed to protect your privacy and sensitive information.Other features include support for custom privacy needs," +
            "steps needed to protect your privacy and sensitive information.Other features include support for custom privacy needs," +
            "steps needed to protect your privacy and sensitive information.Other features include support for custom privacy needs," +
            "steps needed to protect your privacy and sensitive information.Other features include support for custom privacy needs," +
            "steps needed to protect your privacy and sensitive information.Other features include support for custom privacy needs," +
            "steps needed to protect your privacy and sensitive information.Other features include support for custom privacy needs," +
            "steps needed to protect your privacy and sensitive information.Other features include support for custom privacy needs," +
            "steps needed to protect your privacy and sensitive information.Other features include support for custom privacy needs," +
            "steps needed to protect your privacy and sensitive information.Other features include support for custom privacy needs," +
            "steps needed to protect your privacy and sensitive information.Other features include support for custom privacy needs," +
            "steps needed to protect your privacy and sensitive information.Other features include support for custom privacy needs," +
            "steps needed to protect your privacy and sensitive information.Other features include support for custom privacy needs," +
            "steps needed to protect your privacy and sensitive information.Other features include support for custom privacy needs," +
            "steps needed to protect your privacy and sensitive information.Other features include support for custom privacy needs," +
            "steps needed to protect your privacy and sensitive information.Other features include support for custom privacy needs," +
            "steps needed to protect your privacy and sensitive information.Other features include support for custom privacy needs," +
            "steps needed to protect your privacy and sensitive information.Other features include support for custom privacy needs," +
            "steps needed to protect your privacy and sensitive information.Other features include support for custom privacy needs," +
            "steps needed to protect your privacy and sensitive information.Other features include support for custom privacy needs," +
            "steps needed to protect your privacy and sensitive information.Other features include support for custom privacy needs," +
            "steps needed to protect your privacy and sensitive information.Other features include support for custom privacy needs," +
            "steps needed to protect your privacy and sensitive information.Other features include support for custom privacy needs," +
            "steps needed to protect your privacy and sensitive information.Other features include support for custom privacy needs," +
            "steps needed to protect your privacy and sensitive information.Other features include support for custom privacy needs," +
            "steps needed to protect your privacy and sensitive information.Other features include support for custom privacy needs," +
            "steps needed to protect your privacy and sensitive information.Other features include support for custom privacy needs," +
            "steps needed to protect your privacy and sensitive information.Other features include support for custom privacy needs," +
            "steps needed to protect your privacy and sensitive information.Other features include support for custom privacy needs," +
            "steps needed to protect your privacy and sensitive information.Other features include support for custom privacy needs," +
            "end";
    private GeneralCallback mGeneralCallback;


    @Override
    public void channelRegistered(ChannelHandlerContext ctx) throws Exception {
        super.channelRegistered(ctx);


        Log.i(TAG, "channelRegistered: "+Thread.currentThread().getName());
    }

    @Override
    public void channelInactive(ChannelHandlerContext ctx) throws Exception {
        super.channelInactive(ctx);

        Log.i(TAG, "channelInactive: ");

    }

    @Override
    public void channelActive(ChannelHandlerContext ctx) throws Exception {

        mGeneralCallback.onSuccess();
        Log.i(TAG, "channelActive: ");

        //注册成功  发起心跳
//        ctx.channel().writeAndFlush(Unpooled.copiedBuffer("heart".getBytes()));

//        for (int i = 0; i < 1; i++) {
//            ctx.channel().writeAndFlush(sendMsg("heart"));
//        }

        BaseRequest<Register> request = new BaseRequest<>(1, 101, "20114101", new Register(2, "201141"));

        ctx.channel().writeAndFlush(sendMsg(JSON.toJSONString(request)));

        BaseRequest<HeartBeat> heartBeatBaseRequest = new BaseRequest<>(8, 101, "20114101", new HeartBeat(30,new ArrayList<>()));
        ctx.channel().writeAndFlush(sendMsg(JSON.toJSONString(heartBeatBaseRequest)));
//
//        Runnable runnable = new Runnable() {
//            @Override
//            public void run() {
//                BaseRequest<HeartBeat> heartBeatBaseRequest = new BaseRequest<>(8, 101, "20114101", new HeartBeat(30, new ArrayList<>()));
//                ctx.channel().writeAndFlush(sendMsg(JSON.toJSONString(heartBeatBaseRequest)));
//                ctx.executor().schedule(this,20,TimeUnit.SECONDS);
//                Log.i(TAG, "run: "+"send heartbeat");
//            }
//        };
//        ctx.executor().schedule(runnable,20,TimeUnit.SECONDS);

    }

    @Override
    public void channelRead(ChannelHandlerContext ctx, Object msg) throws Exception {
        try{

            Log.i(TAG, "channelRead: "+(String) msg);

//            ctx.executor().schedule(()->{
//                ctx.channel().writeAndFlush(sendMsg("heart"));
//            },2, TimeUnit.SECONDS);

            //以上代码是接收服务端发来的反馈数据//
//            ctx.channel().writeAndFlush(sendMsg("heart"));

//            ctx.close();
        }finally{
            // Discard the received data silently.
            ReferenceCountUtil.release(msg);
        }
    }


    private ByteBuf  sendMsg(String msg)  {

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

    @Override
    public void exceptionCaught(ChannelHandlerContext ctx, Throwable cause) throws Exception {
        Log.e(TAG, "exceptionCaught: "+cause.getMessage());
        ctx.close();

        mGeneralCallback.onFail(cause.getMessage());
    }



    public void  setCallBack(GeneralCallback generalCallback){

        mGeneralCallback = generalCallback;

    }

}
