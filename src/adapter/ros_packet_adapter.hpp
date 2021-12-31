/*********************************************************************************************************************
Copyright (c) 2020 RoboSense
All rights reserved

By downloading, copying, installing or using the software you agree to this license. If you do not agree to this
license, do not download, install, copy or use the software.

License Agreement
For RoboSense LiDAR SDK Library
(3-clause BSD License)

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the
following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following
disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following
disclaimer in the documentation and/or other materials provided with the distribution.

3. Neither the names of the RoboSense, nor Suteng Innovation Technology, nor the names of other contributors may be used
to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*********************************************************************************************************************/

#pragma once

#include "adapter/adapter.hpp"
#include "msg/ros_msg/lidar_packet_ros.h"

namespace robosense
{
namespace lidar
{

#ifdef ROS_FOUND
#include <ros/ros.h>

inline Packet toRsMsg(const rslidar_msgs::rslidarPacket& ros_msg)
{
  Packet rs_msg(1500);
  //rs_msg.timestamp = ros_msg.header.stamp.toSec();
  //rs_msg.seq = ros_msg.header.seq;
  //rs_msg.type = ros_msg.header.type;
  //rs_msg.frame_id = ros_msg.header.frame_id;
  for (size_t i = 0; i < 1500; i++)
  {
    rs_msg.buf_[i] = ros_msg.data[i];
  }
  return std::move(rs_msg);
}

class RosPacketSource : public DriverSource
{ 
public:

  virtual void init(SourceType src_type, const YAML::Node& config);

private:

  void putPacket(const rslidar_msgs::rslidarPacket& msg);

  std::unique_ptr<ros::NodeHandle> nh_;
  ros::Subscriber pkt_sub_;
};

void RosPacketSource::init(SourceType src_type, const YAML::Node& config)
{
  DriverSource::init(src_type, config);

  std::string ros_recv_topic;
  yamlRead<std::string>(config["ros"], "ros_recv_packet_topic", 
      ros_recv_topic, "rslidar_packets");

  nh_ = std::unique_ptr<ros::NodeHandle>(new ros::NodeHandle());
  pkt_sub_ = nh_->subscribe(ros_recv_topic, 1, &RosPacketSource::putPacket, this);
}

void RosPacketSource::putPacket(const rslidar_msgs::rslidarPacket& msg)
{
  driver_ptr_->decodePacket(toRsMsg(msg));
}

inline rslidar_msgs::rslidarPacket toRosMsg(const Packet& rs_msg)
{
  rslidar_msgs::rslidarPacket ros_msg;
  //rs_msg.timestamp = ros_msg.header.stamp.toSec();
  //rs_msg.seq = ros_msg.header.seq;
  //rs_msg.type = ros_msg.header.type;
  //rs_msg.frame_id = ros_msg.header.frame_id;
  for (size_t i = 0; i < rs_msg.buf_.size(); i++)
  {
    ros_msg.data[i] = rs_msg.buf_[i];
  }
  return std::move(ros_msg);
}

class RosPacketDestination : public PacketDestination
{
public:

  virtual void init(const YAML::Node& config);
  virtual void sendPacket(const Packet& msg);
  virtual ~RosPacketDestination() = default;

private:

  std::unique_ptr<ros::NodeHandle> nh_;
  ros::Publisher pub_;
};

inline void RosPacketDestination::init(const YAML::Node& config)
{
  std::string ros_send_topic;
  yamlRead<std::string>(config["ros"], "ros_send_packet_topic", 
      ros_send_topic, "rslidar_packets");

  nh_ = std::unique_ptr<ros::NodeHandle>(new ros::NodeHandle());
  pub_ = nh_->advertise<rslidar_msgs::rslidarPacket>(ros_send_topic, 10);
}

inline void RosPacketDestination::sendPacket(const Packet& msg)
{
  pub_.publish(toRosMsg(msg));
}

#endif  // ROS_FOUND

#ifdef ROS2_FOUND
#include <rclcpp/rclcpp.hpp>

#if 0
inline rslidar_msg::msg::RslidarPacket toRosMsg(const Packet& rs_msg)
{
  rslidar_msg::msg::RslidarPacket ros_msg;
  //ros_msg.header.stamp.sec = (uint32_t)floor(rs_msg.timestamp);
  //ros_msg.header.stamp.nanosec = 
  //  (uint32_t)round((rs_msg.timestamp - ros_msg.header.stamp.sec) * 1e9);
  //ros_msg.header.frame_id = rs_msg.frame_id;
  for (size_t i = 0; i < rs_msg.packet.size(); i++)
  {
    ros_msg.data[i] = rs_msg.packet[i];
  }
  return std::move(ros_msg);
}

class PacketRosAdapter : virtual public AdapterBase
{
public:
  PacketRosAdapter() = default;
  virtual ~PacketRosAdapter();
  void init(const YAML::Node& config);
  void start();
  void regRecvCallback(const std::function<void(const ScanMsg&)>& callback);
  void regRecvCallback(const std::function<void(const PacketMsg&)>& callback);
  void sendScan(const ScanMsg& msg);
  void sendPacket(const PacketMsg& msg);

private:
  void localMsopCallback(const rslidar_msg::msg::RslidarScan::SharedPtr msg);
  void localDifopCallback(const rslidar_msg::msg::RslidarPacket::SharedPtr msg);

private:
  std::shared_ptr<rclcpp::Node> node_ptr_;
  LidarType lidar_type_;
  rclcpp::Publisher<rslidar_msg::msg::RslidarScan>::SharedPtr scan_pub_;
  rclcpp::Publisher<rslidar_msg::msg::RslidarPacket>::SharedPtr packet_pub_;
  rclcpp::Subscription<rslidar_msg::msg::RslidarScan>::SharedPtr scan_sub_;
  rclcpp::Subscription<rslidar_msg::msg::RslidarPacket>::SharedPtr packet_sub_;
  std::vector<std::function<void(const ScanMsg&)>> scan_cb_vec_;
  std::vector<std::function<void(const PacketMsg&)>> packet_cb_vec_;
};

inline PacketRosAdapter::~PacketRosAdapter()
{
  stop();
}

inline void PacketRosAdapter::init(const YAML::Node& config)
{
  node_ptr_.reset(new rclcpp::Node("rslidar_packets_adapter"));

  std::string lidar_type_str;
  yamlRead<std::string>(config["driver"], "lidar_type", lidar_type_str, "RS16");
  lidar_type_ = RSDriverParam::strToLidarType(lidar_type_str);

  int msg_source;
  yamlReadAbort<int>(config, "msg_source", msg_source);
  if (msg_source == MsgSource::MSG_FROM_ROS_PACKET)
  {
    std::string ros_recv_topic;
    yamlRead<std::string>(config["ros"], "ros_recv_packet_topic", ros_recv_topic, "rslidar_packets");

    packet_sub_ = node_ptr_->create_subscription<rslidar_msg::msg::RslidarPacket>(
        ros_recv_topic + "_difop", 10,
        [this](const rslidar_msg::msg::RslidarPacket::SharedPtr msg) { localDifopCallback(msg); });
    scan_sub_ = node_ptr_->create_subscription<rslidar_msg::msg::RslidarScan>(
        ros_recv_topic, 10, [this](const rslidar_msg::msg::RslidarScan::SharedPtr msg) { localMsopCallback(msg); });
  }

  bool send_packet_ros;
  yamlRead<bool>(config, "send_packet_ros", send_packet_ros, false);
  if (send_packet_ros)
  {
    std::string ros_send_topic;
    yamlRead<std::string>(config["ros"], "ros_send_packet_topic", ros_send_topic, "rslidar_packets");

    packet_pub_ = node_ptr_->create_publisher<rslidar_msg::msg::RslidarPacket>(ros_send_topic + "_difop", 10);
    scan_pub_ = node_ptr_->create_publisher<rslidar_msg::msg::RslidarScan>(ros_send_topic, 10);
  }
}

inline void PacketRosAdapter::start()
{
  std::thread t([this]() { rclcpp::spin(node_ptr_); });
  t.detach();
}

inline void PacketRosAdapter::regRecvCallback(const std::function<void(const ScanMsg&)>& callback)
{
  scan_cb_vec_.emplace_back(callback);
}

inline void PacketRosAdapter::regRecvCallback(const std::function<void(const PacketMsg&)>& callback)
{
  packet_cb_vec_.emplace_back(callback);
}

inline void PacketRosAdapter::sendScan(const ScanMsg& msg)
{
  scan_pub_->publish(toRosMsg(msg));
}

inline void PacketRosAdapter::sendPacket(const PacketMsg& msg)
{
  packet_pub_->publish(toRosMsg(msg));
}

inline void PacketRosAdapter::localMsopCallback(const rslidar_msg::msg::RslidarScan::SharedPtr msg)
{
  for (auto& cb : scan_cb_vec_)
  {
    cb(toRsMsg(lidar_type_, PktType::MSOP, *msg));
  }
}

inline void PacketRosAdapter::localDifopCallback(const rslidar_msg::msg::RslidarPacket::SharedPtr msg)
{
  for (auto& cb : packet_cb_vec_)
  {
    cb(toRsMsg(lidar_type_, PktType::DIFOP, *msg));
  }
}

#endif
#endif  // ROS2_FOUND

}  // namespace lidar
}  // namespace robosense

