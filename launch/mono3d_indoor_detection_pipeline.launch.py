# Copyright (c) 2022，Horizon Robotics.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

import os

from launch import LaunchDescription
from launch_ros.actions import Node

from launch.actions import IncludeLaunchDescription
from launch.launch_description_sources import PythonLaunchDescriptionSource
from ament_index_python import get_package_share_directory
from ament_index_python.packages import get_package_prefix

def generate_launch_description():
    web_service_launch_include = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(
            os.path.join(
                get_package_share_directory('websocket'),
                'launch/hobot_websocket_service.launch.py'))
    )
    
    # 本地图片发布
    fb_node = Node(
        package='hobot_image_publisher',
        executable='hobot_image_pub',
        output='screen',
        parameters=[
            {"image_source": "./config/images/3d_detection.png"},
            {"image_format": "png"},
            {"msg_pub_topic_name": "/hbmem_img"},
            {"fps": 5}
        ],
        arguments=['--ros-args', '--log-level', 'error']
    )

    # mipi cam图片发布
    mipi_node = Node(
        package='mipi_cam',
        executable='mipi_cam',
        output='screen',
        parameters=[
            {"out_format": "nv12"},
            {"image_width": 1920},
            {"image_height": 1080},
            {"io_method": "shared_mem"},
            {"video_device": "F37"}
        ],
        arguments=['--ros-args', '--log-level', 'error']
    )

    # usb cam图片发布
    usb_node = Node(
        package='hobot_usb_cam',
        executable='hobot_usb_cam',
        name='hobot_usb_cam',
        parameters=[
                    {"frame_id": "default_usb_cam"},
                    {"image_height": 1920},
                    {"image_width": 1080},
                    {"zero_copy": False},
                    {"video_device": "/dev/video8"}
                    ],
        arguments=['--ros-args', '--log-level', 'error']
    )

    # nv12->jpeg图片编码&发布
    jpeg_codec_node = Node(
        package='hobot_codec',
        executable='hobot_codec_republish',
        output='screen',
        parameters=[
            {"channel": 1},
            {"in_mode": "shared_mem"},
            {"in_format": "nv12"},
            {"out_mode": "ros"},
            {"out_format": "jpeg"},
            {"sub_topic": "/hbmem_img"},
            {"pub_topic": "/image"}
        ],
        arguments=['--ros-args', '--log-level', 'error']
    )

    # jpeg->nv12图片解码&发布
    nv12_codec_node = Node(
        package='hobot_codec',
        executable='hobot_codec_republish',
        output='screen',
        parameters=[
            {"channel": 1},
            {"in_mode": "ros"},
            {"in_format": "jpeg"},
            {"out_mode": "shared_mem"},
            {"out_format": "nv12"},
            {"sub_topic": "/image"},
            {"pub_topic": "/hbmem_img"}
        ],
        arguments=['--ros-args', '--log-level', 'error']
    )


    # 算法检测
    mono3d_det_node = Node(
        package='mono3d_indoor_detection',
        executable='mono3d_indoor_detection',
        output='screen',
        parameters=[
            {"config_file_path": "./config"},
            {"ai_msg_pub_topic_name": "ai_msg_3d_detection"},
            {"dump_render_img": 0},
            {"shared_mem": 1}
        ],
        arguments=['--ros-args', '--log-level', 'info']
    )


    # web展示
    web_node = Node(
        package='websocket',
        executable='websocket',
        output='screen',
        parameters=[
            {"image_topic": "/image"},
            {"image_type": "mjpeg"},
            {"smart_topic": "/ai_msg_3d_detection"}
        ],
        arguments=['--ros-args', '--log-level', 'error']
    )

    camera_type = os.getenv('CAM_TYPE')
    print("camera_type is ", camera_type)
    cam_node = mipi_node
    camera_type_mipi = True
    if camera_type == "usb":
        print("using usb cam")
        cam_node = usb_node
        camera_type_mipi = False
    elif camera_type == "mipi":
        print("using mipi cam")
        cam_node = mipi_node
        camera_type_mipi = True
    elif camera_type == "fb":
        print("using feedback")
        cam_node = fb_node
        camera_type_mipi = True
    else:
        print("invalid camera_type ", camera_type, ", which is set with export CAM_TYPE=usb/mipi/fb, using default mipi cam")
        cam_node = mipi_node
        camera_type_mipi = True

    if camera_type_mipi:
        return LaunchDescription([
            web_service_launch_include,
            # 图片发布
            cam_node,
            # 图片编解码&发布
            jpeg_codec_node,
            # 启动算法
            mono3d_det_node,
            # 启动web展示
            web_node
        ])
    else:
        return LaunchDescription([
            web_service_launch_include,
            # 图片发布
            cam_node,
            # 图片编解码&发布
            nv12_codec_node,
            # 启动算法
            mono3d_det_node,
            # 启动web展示
            web_node
        ])
 