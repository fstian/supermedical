package com.witted.service;

import android.util.Log;

import com.alibaba.fastjson.JSON;
import com.witted.service.model.BaseRequest;
import com.witted.service.model.HeartBeat;
import com.witted.service.model.Register;

import java.io.UnsupportedEncodingException;
import java.util.ArrayList;

import io.netty.buffer.ByteBuf;
import io.netty.buffer.Unpooled;
import io.netty.channel.ChannelHandlerContext;
import io.netty.channel.ChannelInboundHandlerAdapter;
import io.netty.handler.timeout.IdleState;
import io.netty.handler.timeout.IdleStateEvent;

public class HeartBeatHandler extends ChannelInboundHandlerAdapter {
    
    private static final String TAG=HeartBeatHandler.class.getName();


    int TRY_TIMES=3;

    int currentTimes=0;

    @Override
    public void userEventTriggered(ChannelHandlerContext ctx, Object evt) throws Exception {
        super.userEventTriggered(ctx, evt);
        Log.i(TAG, "userEventTriggered: ");

        if (evt instanceof IdleStateEvent) {
            IdleStateEvent event = (IdleStateEvent) evt;
            if(event.state()== IdleState.WRITER_IDLE){
                currentTimes++;
                Log.i(TAG, "userEventTriggered: "+"___WRITER_IDLE");

            }
        }
        BaseRequest<HeartBeat> request = new BaseRequest<>(8, 101, "20114101", new HeartBeat(30,new ArrayList<>()));
        ctx.channel().writeAndFlush(sendMsg(JSON.toJSONString(request)));

    }

    @Override
    public void channelActive(ChannelHandlerContext ctx) throws Exception {
        super.channelActive(ctx);
    }

    @Override
    public void channelInactive(ChannelHandlerContext ctx) throws Exception {
        super.channelInactive(ctx);
        Log.i(TAG, "channelInactive: ");

        ctx.fireChannelActive();
    }

    @Override
    public void channelRead(ChannelHandlerContext ctx, Object msg) throws Exception {
        super.channelRead(ctx, msg);
        Log.i(TAG, "channelRead: "+msg);
        
    }

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
