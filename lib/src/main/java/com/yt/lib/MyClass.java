package com.yt.lib;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.net.ServerSocket;
import java.net.Socket;

public class MyClass {



    public static void main(String[] args){

        int count=0;
        try {
            ServerSocket serverSocket = new ServerSocket(17878);

            while (true){
                count++;
                Socket accept = serverSocket.accept();
                new Thread(new ReceiveRunable(accept),"threadname"+count).start();
            }

        } catch (IOException e) {
            e.printStackTrace();
        }


    }

    static class  ReceiveRunable implements Runnable {

        private final Socket mSocket;

        public ReceiveRunable(Socket socket) {
            mSocket = socket;
        }

        public void  sendMsg(String text){

            try {
                mSocket.getOutputStream().write((text+"\n").getBytes());
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
                int len=0;
                String text;

                while ((len=is.read(bytes))>0){
                    System.out.println("lensize"+len);
                    if(len==89){
                        sendMsg("aaaa");
                    }
                }


            } catch (IOException e) {
                e.printStackTrace();
            }


        }
    }

}
