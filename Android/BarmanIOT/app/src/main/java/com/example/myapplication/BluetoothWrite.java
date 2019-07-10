package com.example.myapplication;

import android.bluetooth.BluetoothSocket;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import android.os.Handler;

public class BluetoothWrite extends Thread{

    private final InputStream mmInStream;
    private final OutputStream mmOutStream;
    Handler bluetoothIn;
    final int handlerState = 0;
    private TragosActivity tragos;
    private static BluetoothWrite obj;

    public BluetoothWrite(BluetoothSocket socket , TragosActivity tragos)
    {
        InputStream tmpIn = null;
        OutputStream tmpOut = null;
        try
        {
            tmpIn = socket.getInputStream();
            tmpOut = socket.getOutputStream();
        } catch (IOException e) { }

        mmInStream = tmpIn;
        mmOutStream = tmpOut;
        this.tragos = tragos;
    }

    public static  BluetoothWrite getInstance(BluetoothSocket socket , TragosActivity tragos) {
        if(obj == null) {
            obj = new BluetoothWrite(socket,tragos);
        }
        return obj;
    }

    //metodo run del hilo, que va a entrar en una espera activa para recibir los msjs del HC05
    public void run()
    {
        byte[] buffer = new byte[256];
        int bytes;

        //el hilo secundario se queda esperando mensajes del HC05
        while (true)
        {
            try
            {
                //se leen los datos del Bluethoot
                bytes = mmInStream.read(buffer);
                String readMessage = new String(buffer, 0, bytes);

                //se muestran en el layout de la activity, utilizando el handler del hilo
                // principal antes mencionado
                bluetoothIn.obtainMessage(handlerState, bytes, -1, readMessage).sendToTarget();
            } catch (IOException e) {
                break;
            }
        }
    }


    //write method
    public void write(String input) {
        byte[] msgBuffer = input.getBytes();           //converts entered String into bytes
        try {
            mmOutStream.write(msgBuffer);                //write bytes over BT connection via outstream
        } catch (IOException e) {
            //if you cannot write, close the application
            tragos.showToast("La conexion fallo");
            tragos.finish();
        }
    }
}
