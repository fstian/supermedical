package com.yt.lib;


import io.netty.buffer.ByteBuf;
import io.netty.buffer.Unpooled;
import io.netty.channel.ChannelFuture;
import io.netty.channel.ChannelFutureListener;
import io.netty.channel.ChannelHandlerContext;
import io.netty.channel.ChannelInboundHandlerAdapter;
import io.netty.util.CharsetUtil;
import io.netty.util.ReferenceCountUtil;

/**
 * 服务端业务处理类
 * (编写主要的业务逻辑)
 * @author huangjianfei
 */
public class ServerHandler extends ChannelInboundHandlerAdapter {
    
    private static final String  TAG=ServerHandler.class.getName();

    @Override
    public void channelRegistered(ChannelHandlerContext ctx) throws Exception {
        super.channelRegistered(ctx);
        System.out.println("receive"+"channelRegistered" +ctx.channel().hashCode());

    }

    @Override
    public void channelUnregistered(ChannelHandlerContext ctx) throws Exception {
        super.channelUnregistered(ctx);
        System.out.println("receive"+"channelUnregistered"+ctx.channel().hashCode() );

    }

    @Override
    public void channelActive(ChannelHandlerContext ctx) throws Exception {
        super.channelActive(ctx);
        System.out.println("receive"+"channelActive" +ctx.channel().hashCode());
    }

    /**
     * 每当从客户端收到新的数据时，这个方法会在收到消息时被调用
     * ByteBuf是一个引用计数对象，这个对象必须显示地调用release()方法来释放。
     * 请记住处理器的职责是释放所有传递到处理器的引用计数对象。
     */


    @Override
    public void channelRead(ChannelHandlerContext ctx, Object msg) throws Exception {
        System.out.println("receive"+"channelRead" +ctx.channel().hashCode());

        String name = ctx.name();

        try{

            String request = (String) msg;
            System.out.println("receive msg"+ request);

            ChannelFuture channelFuture = ctx.channel().writeAndFlush(Unpooled.copiedBuffer("Netty test \n", CharsetUtil.UTF_8));


        }finally{
            // Discard the received data silently.
            ReferenceCountUtil.release(msg);
        }
    }

    /**
     * exceptionCaught()事件处理方法是当出现Throwable对象才会被调用
     * 即当Netty由于IO错误或者处理器在处理事件时抛出的异常时
     */
    @Override
    public void exceptionCaught(ChannelHandlerContext ctx, Throwable cause) throws Exception
    {
        // Close the connection when an exception is raised.

        System.out.println("receive msg"+ cause.getMessage());
        cause.printStackTrace();
        ctx.close();
    }

}