/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2006,2007 INRIA
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
 */

#include "ns3/core-module.h"
#include "ns3/mobility-module.h"
#include "ns3/netanim-module.h"
using namespace ns3;

/**
 * Function called when there is a course change
 * \param context event context
 * \param mobility a pointer to the mobility model
 */
static void
CourseChange(std::string context, Ptr<const MobilityModel> mobility)
{
    Vector pos = mobility->GetPosition();
    Vector vel = mobility->GetVelocity();
    std::cout << Simulator::Now() << ", model=" << mobility << ", POS: x=" << pos.x << ", y=" << pos.y
        << ", z=" << pos.z << "; VEL:" << vel.x << ", y=" << vel.y
        << ", z=" << vel.z << std::endl;
}

int main(int argc, char* argv[])
{
    Config::SetDefault("ns3::RandomWalk2dMobilityModel::Mode", StringValue("Time"));//改变当前速度和方向条件的模式是时间模式
    Config::SetDefault("ns3::RandomWalk2dMobilityModel::Time", StringValue("2s"));//时延
    Config::SetDefault("ns3::RandomWalk2dMobilityModel::Speed", StringValue("ns3::ConstantRandomVariable[Constant=1.0]"));//速度定为1
    Config::SetDefault("ns3::RandomWalk2dMobilityModel::Bounds", StringValue("0|200|0|200"));//边界范围

    CommandLine cmd(__FILE__);
    cmd.Parse(argc, argv);

    NodeContainer c;
    c.Create(100);//创建100个点

    MobilityHelper mobility;//创建一个移动助手类
    mobility.SetPositionAllocator("ns3::RandomDiscPositionAllocator", //位置分配器
        "X", StringValue("100.0"),//圆心的X坐标
        "Y", StringValue("100.0"),//圆心的Y坐标
        "Rho", StringValue("ns3::UniformRandomVariable[Min=0|Max=30]"));//圆的半径在0和30之间随机
    mobility.SetMobilityModel("ns3::RandomWalk2dMobilityModel",//调用RandomWalk2dMobilityModel.cc  二维随机游走模型
        "Mode", StringValue("Distance"),//改变当前速度和方向条件的模式是时间模式
        "Time", StringValue("2s"),//时延
        "Speed", StringValue("ns3::ConstantRandomVariable[Constant=1.0]"),//速度定为1
        "Bounds", StringValue("0|200|0|200"));//边界范围
    mobility.InstallAll();
    Config::Connect("/NodeList/*/$ns3::MobilityModel/CourseChange",
        MakeCallback(&CourseChange));
    AnimationInterface anim("random_walk.xml");
    Simulator::Stop(Seconds(100.0));

    Simulator::Run();

    Simulator::Destroy();
    return 0;
}

