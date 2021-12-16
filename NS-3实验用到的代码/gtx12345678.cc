/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2009 University of Washington
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

//
// This program configures a grid (default 5x5) of nodes on an
// 802.11b physical layer, with
// 802.11b NICs in adhoc mode, and by default, sends one packet of 1000
// (application) bytes to node 1.
//
// The default layout is like this, on a 2-D grid.
//
// n20  n21  n22  n23  n24
// n15  n16  n17  n18  n19
// n10  n11  n12  n13  n14
// n5   n6   n7   n8   n9
// n0   n1   n2   n3   n4
//
// the layout is affected by the parameters given to GridPositionAllocator;
// by default, GridWidth is 5 and numNodes is 25..
//
// There are a number of command-line options available to control
// the default behavior.  The list of available command-line options
// can be listed with the following command:
// ./waf --run "wifi-simple-adhoc-grid --help"
//
// Note that all ns-3 attributes (not just the ones exposed in the below
// script) can be changed at command line; see the ns-3 documentation.
//
// For instance, for this configuration, the physical layer will
// stop successfully receiving packets when distance increases beyond
// the default of 500m.
// To see this effect, try running:
//
// ./waf --run "wifi-simple-adhoc --distance=500"
// ./waf --run "wifi-simple-adhoc --distance=1000"
// ./waf --run "wifi-simple-adhoc --distance=1500"
//
// The source node and sink node can be changed like this:
//
// ./waf --run "wifi-simple-adhoc --sourceNode=20 --sinkNode=10"
//
// This script can also be helpful to put the Wifi layer into verbose
// logging mode; this command will turn on all wifi logging:
//
// ./waf --run "wifi-simple-adhoc-grid --verbose=1"
//
// By default, trace file writing is off-- to enable it, try:
// ./waf --run "wifi-simple-adhoc-grid --tracing=1"
//
// When you are done tracing, you will notice many pcap trace files
// in your directory.  If you have tcpdump installed, you can try this:
//
// tcpdump -r wifi-simple-adhoc-grid-0-0.pcap -nn -tt
//

#include <stdlib.h>//C语言标准库函数定义

//示例代码
//路径ns-allinone-3.34/ns-3.34/examples/wireless/wifi-simple-adhoc-grid.cc
//参考拓扑模型、移动性建模、信道建模、协议层建模等
#include "ns3/command-line.h"
#include "ns3/config.h"
#include "ns3/uinteger.h"
#include "ns3/double.h"
#include "ns3/string.h"
#include "ns3/log.h"
#include "ns3/yans-wifi-helper.h"
#include "ns3/mobility-helper.h"
#include "ns3/ipv4-address-helper.h"
#include "ns3/yans-wifi-channel.h"
#include "ns3/mobility-model.h"
#include "ns3/olsr-helper.h"//OLSR协议
#include "ns3/ipv4-static-routing-helper.h"
#include "ns3/ipv4-list-routing-helper.h"
#include "ns3/internet-stack-helper.h"
#include "ns3/aodv-module.h"//AODV协议

//------------------------------------------------------------
//-- 统计模块头文件
//参考ns-3.34/examples/stats/wifi-example-sim.cc
#include <ctime>
#include <sstream>
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/mobility-module.h"
#include "ns3/internet-module.h"
#include "ns3/stats-module.h"
#include "ns3/yans-wifi-helper.h"
#include "ns3/temp.h"//编译好的统计模块头文件
//参考ns-3.34/examples/stats/wifi-example-apps.cc和wifi-example-apps.h
//直接调用wifi-example-apps.h会报错,需要用新建库函数的方式
#include "ns3/netanim-module.h"//可视化NetAnim模块


using namespace ns3;
using namespace std;//C++


NS_LOG_COMPONENT_DEFINE ("gtx12345678");

//------------------------------------------------------------
//-- 2.TxCallback函数
//功能：1、输出信息；2、递增1计数;
void TxCallback (Ptr<CounterCalculator<uint32_t> > datac,
                 std::string path, Ptr<const Packet> packet) {
  NS_LOG_INFO ("Sent frame counted in " <<
               datac->GetKey ());
  datac->Update ();
  
}

int main (int argc, char *argv[])
{
  //统计模块
  string format ("omnet");//指明输出文件类型为omnet

  string experiment("wifi-distance-test"); //实验名称，按需命名；
  string strategy("wifi-default");//要检查的代码或参数，本例中它是固定的。
  string input;//具体问题，本例中是两个节点之间的距离。
  string runID;//本次实验的唯一标识符，其信息在以后的分析中被标记，以供识别。

  std::string phyMode ("DsssRate1Mbps");
  uint32_t packetSize = 1000; // 发包大小
  uint32_t numPackets = 1;
  uint32_t numNodes = 100;  //定义节点数量
  uint32_t sinkNode ;
  uint32_t sourceNode ;
  uint32_t e_num = 90;
  double interval = 1.0; // 间隔单位为1秒
  double distance = 500;  // 距离为500米
  bool verbose = false;
  bool tracing = false;
  
  CommandLine cmd;
  cmd.AddValue ("phyMode", "Wifi Phy mode", phyMode);
  cmd.AddValue ("distance", "distance (m)", distance);
  cmd.AddValue ("packetSize", "size of application packet sent", packetSize);
  cmd.AddValue ("numPackets", "number of packets generated", numPackets);
  cmd.AddValue ("interval", "interval (seconds) between packets", interval);
  cmd.AddValue ("verbose", "turn on all WifiNetDevice log components", verbose);
  cmd.AddValue ("tracing", "turn on ascii and pcap tracing", tracing);
  cmd.AddValue ("numNodes", "number of nodes", numNodes);
  cmd.AddValue ("sinkNode", "Receiver node number", sinkNode);
  cmd.AddValue ("format", "Format to use for data output.",format);
  cmd.AddValue ("sourceNode", "Sender node number", sourceNode);
  cmd.AddValue ("experiment", "Identifier for experiment.",experiment);
  cmd.AddValue ("strategy", "Identifier for strategy.",strategy);
  cmd.AddValue ("run", "Identifier for run.",runID);
  cmd.AddValue ("e_num", "the node num of energy", e_num);
  cmd.Parse (argc, argv);

  //input具体定义
 {
    stringstream sstr ("");
    sstr << distance;
    input = sstr.str ();
  }

  //建立100个节点
  NodeContainer c;
  c.Create (numNodes);

  //下面的helper用来帮助我们整合所需要的网络适配器
  WifiHelper wifi;
  if (verbose)
    {
      wifi.EnableLogComponents ();  // 打开Wifi日志记录
    }

  //物理层参数设置
  YansWifiPhyHelper wifiPhy;

  wifiPhy.Set("TxGain", DoubleValue(0));//传输增益
  wifiPhy.Set ("RxGain", DoubleValue (10) );//接收增益
  wifiPhy.Set ("CcaEdThreshold", DoubleValue (-62.0));//阈值检测，高于阈值即可以在PHY层被检测到
  wifiPhy.Set ("TxPowerStart", DoubleValue(16.0206));//最小发射功率
  wifiPhy.Set ("TxPowerEnd", DoubleValue(16.0206));//最大发射功率

  // pcap跟踪记录：支持802.11b协议及其扩展内容
  wifiPhy.SetPcapDataLinkType (WifiPhyHelper::DLT_IEEE802_11_RADIO);

  //WiFi信道参数设置
  YansWifiChannelHelper wifiChannel;
  wifiChannel.SetPropagationDelay ("ns3::ConstantSpeedPropagationDelayModel");
  //Friis衰落信道
  //wifiChannel.AddPropagationLoss ("ns3::FriisPropagationLossModel");
  //LogDistance衰落信道
  //可设置参数参考官方文档
  wifiChannel.AddPropagationLoss ("ns3::LogDistancePropagationLossModel"
                                  ,"ReferenceDistance",DoubleValue(100)
                                  ,"ReferenceLoss",DoubleValue(86.67));
  wifiPhy.SetChannel (wifiChannel.Create ());

  // Mac层参数设置
  WifiMacHelper wifiMac;

  wifi.SetStandard (WIFI_STANDARD_80211b);//802.11b
  wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager",
                                "DataMode",StringValue (phyMode),
                                "ControlMode",StringValue (phyMode));
  //Adhoc模式设置
  wifiMac.SetType ("ns3::AdhocWifiMac");
  NetDeviceContainer devices = wifi.Install (wifiPhy, wifiMac, c);

  MobilityHelper mobility;
  mobility.SetPositionAllocator ("ns3::GridPositionAllocator",//网格位置分配
                                 "MinX", DoubleValue (0.0),
                                 "MinY", DoubleValue (0.0),
                                 "DeltaX", DoubleValue (distance),//间距
                                 "DeltaY", DoubleValue (distance),
                                 "GridWidth", UintegerValue (10),//一行节点数量
                                 "LayoutType", StringValue ("RowFirst"));

  //静止运行模型
  //  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  //随机游走2D运动模型
  mobility.SetMobilityModel ("ns3::RandomWalk2dMobilityModel",
                             "Bounds", RectangleValue (Rectangle (-20000, 20000, -20000, 20000)),
                             "Distance",DoubleValue(1000));
  
  mobility.Install (c);



  /** 网络层路由协议 **/
  /***************************************************************************/
 // Enable OLSR & AODV
  OlsrHelper olsr;//OLSR协议对象
  AodvHelper aodv;//AODV协议对象
  Ipv4StaticRoutingHelper staticRouting;//建立StaticRouting对象

  Ipv4ListRoutingHelper list;//建立routinglist对象，Add()将路由协议添加至路由表中
  list.Add (staticRouting, 0);//设置优先级为0
  //list.Add (olsr, 10);//设置OLSR优先级为10
  list.Add(aodv,10); //设置AODV优先级为10


  InternetStackHelper internet; //创建路由列表
  internet.SetRoutingHelper (list); //list将作用于Install ()
  //internet.SetRoutingHelper(olsr); //另一种部署路由协议的方法
  internet.Install (c);//安装协议栈

  //ipv4协议分配ip
  Ipv4AddressHelper ipv4;
  NS_LOG_INFO ("Assign IP Addresses.");
  ipv4.SetBase ("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer i = ipv4.Assign (devices);


  //------------------------------------------------------------
  //-- 4.配置要收集的数据和统计信息
  //--------------------------------------------

  //存储本次运行的信息，用于分类多个实验中的数据，可以根据需要添加更多的数据块。
  DataCollector data;
  data.DescribeRun(experiment,
	  strategy,
	  input,
	  runID);

  // 添加需要的信息
  data.AddMetadata("author", "tjkopena");


  /***************************统计模块******************************/
  //实验
  //设置节点0-9为source节点，节点90-99为sink节点，即统计10对节点的收发。
  //且节点0发送数据包的目的地为节点90的IP地址，以此类推。
  //其中各个节点sender的开始时间不应相同（若相同，会发包冲突严重），
  //且节点间开始时间的间隔会对实际发包数产生影响；

  //------------------------------------------------------------
  //-- 3.安装发送器和接收器
  //--------------------------------------------
  NS_LOG_INFO ("Create traffic source & sink.");
  /*设置节点0-9为sender*/
  /*设置节点90-99为sink，且与节点0-9一一对应*/
  for (int iter = 0; iter < 10; iter++)
  {
	  Ptr<Node> appSource = NodeList::GetNode(iter);
	  Ptr<Sender> sender = CreateObject<Sender>();
	  appSource->AddApplication(sender);
	  sender->SetStartTime(Seconds(iter + 1));

	  Ptr<Node> appSink = NodeList::GetNode(90 + iter);
	  Ptr<Receiver> receiver = CreateObject<Receiver>();
	  appSink->AddApplication(receiver);
	  receiver->SetStartTime(Seconds(0));
	  string s = "10.1.1.9" + std::to_string(iter);
	  /*print IP*/
          char ipv4add[10];
	  strcpy(ipv4add, s.c_str());
	  /*std::cout << ipv4add << std::endl;*/

	  Config::Set("/NodeList/" + std::to_string(iter) + "/ApplicationList/*/$Sender/Destination",
		  Ipv4AddressValue(ipv4add));//"192.168.0."+"2"+std::to_string(90+iter)


  //------------------------------------------------------------
  //-- 5.统计source节点发帧数
  //实际的观察和计算由ns3::DataCalculator对象完成
  //此处计数器通过上文定义的函数TxCallback()连接到WIFI MAC层的trace信号
  //调用统计模块计数器，记录节点0-9发送的帧数。

  	Ptr<CounterCalculator<uint32_t> > totalTx =
    		CreateObject<CounterCalculator<uint32_t> >();
  	totalTx->SetKey ("wifi-tx-frames");
  	totalTx->SetContext ("node["+std::to_string(iter)+"]");
  	Config::Connect ("/NodeList/"+std::to_string(iter)+"/DeviceList/*/$ns3::WifiNetDevice/Mac/MacTx",
                    MakeBoundCallback (&TxCallback, totalTx));
  	data.AddDataCalculator (totalTx);
 
	//------------------------------------------------------------
	//-- 6.统计sink节点收帧数
	//这里使用适配类的方法将计数器直接连接到WiFi MAC生成的trace信号。
	//调用统计模块计数器，记录节点90-99接收的帧数。
  	Ptr<PacketCounterCalculator> totalRx =
    		CreateObject<PacketCounterCalculator>();
  	totalRx->SetKey ("wifi-rx-frames");
  	totalRx->SetContext ("node["+std::to_string(90+iter)+"]");
  	Config::Connect ("/NodeList/"+std::to_string(90+iter)+"/DeviceList/*/$ns3::WifiNetDevice/Mac/MacRx",
                   	 MakeCallback (&PacketCounterCalculator::PacketUpdate,
                         	         totalRx));
  	data.AddDataCalculator (totalRx);
  
	//------------------------------------------------------------
	//-- 7.统计source节点发包数，
	//计数器直接连接到sender类提供的trace信号。 
	//调用统计模块计数器，记录节点0-9发包数。
  	Ptr<PacketCounterCalculator> appTx =
     		CreateObject<PacketCounterCalculator>();
  	appTx->SetKey ("sender-tx-packets");
  	appTx->SetContext ("node["+std::to_string(iter)+"]");
  	Config::Connect ("/NodeList/"+std::to_string(iter)+"/ApplicationList/*/$Sender/Tx",
          	          MakeCallback (&PacketCounterCalculator::PacketUpdate,
                                  appTx));
  	data.AddDataCalculator (appTx);


	//------------------------------------------------------------
	//-- 8.统计sink节点收包数，
	//这里没有采用Trace机制，而是直接利用Receiver::SetCounter进行操作，
	//通过SetCounter显示类型转换，将appRx赋值给Receiver内部计数器，
	//从而实现计数。
	//调用统计模块计数器，记录节点90收包数
   Ptr<CounterCalculator<> > appRx =
     		CreateObject<CounterCalculator<> >();
  	appRx->SetKey ("receiver-rx-packets");
  	appRx->SetContext ("node["+std::to_string(90+iter)+"]");
  	receiver->SetCounter (appRx);
  	data.AddDataCalculator (appRx);

	//------------------------------------------------------------
	//-- 9.统计节点的传输时延
	//这里不采用Trace机制，
	//直接利用Receiver::SetDelayTracker记录传输时延最值/平均值等。
	//调用统计模块时延计算，记录节点90的传输时延，包括最小、最大和平均时延。

  	Ptr<TimeMinMaxAvgTotalCalculator> delayStat =
     	CreateObject<TimeMinMaxAvgTotalCalculator>();
	delayStat->SetKey ("delay");
  	delayStat->SetContext (".");
  	receiver->SetDelayTracker (delayStat);
  	data.AddDataCalculator (delayStat);
  }

  


  if (tracing == true)
  {
	  AsciiTraceHelper ascii;
	  wifiPhy.EnableAsciiAll(ascii.CreateFileStream("wifi-simple-adhoc-grid.tr"));
	  wifiPhy.EnablePcap("wifi-simple-adhoc-grid", devices);
	  // Trace routing tables
	  Ptr<OutputStreamWrapper> routingStream = Create<OutputStreamWrapper>("wifi-simple-adhoc-grid.routes", std::ios::out);
	  aodv.PrintRoutingTableAllEvery(Seconds(2), routingStream);
	  Ptr<OutputStreamWrapper> neighborStream = Create<OutputStreamWrapper>("wifi-simple-adhoc-grid.neighbors", std::ios::out);
	  aodv.PrintNeighborCacheAllEvery(Seconds(2), neighborStream);
	  //实现一个仅显示转发事件的IP级别的路径
  }

 // NS_LOG_UNCOND ("Testing from node " << sourceNode << " to " << sinkNode << " with grid distance " << distance);
 
  
  //可视化结果输出
  AnimationInterface anim("gtx12345678.xml");
  anim.SetMaxPktsPerTraceFile(99999999999999);//解决NetAnim"Max Packets per trace file exceeded"问题
  
  Simulator::Stop (Seconds (33.0));
  Simulator::Run ();


  //------------------------------------------------------------
  //-- 10.统计结果输出
  //--------------------------------------------

  //根据所选的输出模式输出：1为omnet（另一个网络仿真器）公式，2为数据库格式（sql语言数据库编写）；
  Ptr<DataOutputInterface> output = 0;
  if (format == "omnet") {
      NS_LOG_INFO ("Creating omnet formatted data output.");//创建omnet格式的数据输出
      output = CreateObject<OmnetDataOutput>();
    } else if (format == "db") {
    #ifdef STATS_HAS_SQLITE3
      NS_LOG_INFO ("Creating sqlite formatted data output.");//创建sqlite格式的数据输出
      output = CreateObject<SqliteDataOutput>();
    #endif
    } else {
      NS_LOG_ERROR ("Unknown output format " << format);//未知的输出格式
    }


  if (output != 0)
    output->Output (data);


  Simulator::Destroy ();

  return 0;
}

