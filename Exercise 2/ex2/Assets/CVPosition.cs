using System;
using System.Collections;
using System.Collections.Generic;
using System.Net;
using System.Net.Sockets;
using System.Text;
using System.Threading;
using UnityEngine;

public class CVPosition : MonoBehaviour {

    public int receivingPort = 12345;

    public int maxWebcamWidth = 640;
    public int maxWebcamHeight = 480;

    private float x;
    private float y;
    private float r;

    private string lastPacket;
    private Thread listenerThread;
    
	// Use this for initialization
	void Start () {
        StartUDPListener();
    }

    void OnDisable()
    {
        listenerThread.Abort();
    }

    // Update is called once per frame
    void Update () {
        //Debug.Log("[x: " + x + ", y: " + y + ", r: " + r + "]");

        float sceneX = (x - maxWebcamWidth / 2 ) * 1/50;
        float sceneY = (y - maxWebcamHeight / 2) * 1/50;

        transform.position = new Vector3(-sceneX, -sceneY);
        transform.localScale = new Vector3(1, 1, 1); 
    }

    private void StartUDPListener()
    {
        if (listenerThread != null) return;

        listenerThread = new Thread(new ThreadStart(ReceivePackets));
        listenerThread.IsBackground = true;
        listenerThread.Start();
    }

    private void ReceivePackets()
    {
        UdpClient client = new UdpClient(receivingPort);
        while (true)
        {
            try
            {
                IPEndPoint anyIP = new IPEndPoint(IPAddress.Any, 0);
                byte[] data = client.Receive(ref anyIP);

                this.x = BitConverter.ToSingle(data,  0);
                this.y = BitConverter.ToSingle(data,  4);
                this.r = BitConverter.ToSingle(data,  8);

                //Debug.Log("'[x: " + x + ", y: " + y + ", r: " + r + "]");
            }
            catch (Exception err)
            {
                Debug.LogError(err.ToString());
            }
        }
    }

    public static string ByteArrayToString(byte[] ba)
    {
        return BitConverter.ToString(ba).Replace("-", "");
    }
}
