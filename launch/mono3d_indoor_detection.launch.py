# Copyright (c) 2024，D-Robotics.
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
from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument, SetEnvironmentVariable
from launch.substitutions import LaunchConfiguration
from launch_ros.actions import Node


def generate_launch_description():
    return LaunchDescription([
        Node(
            package='mono3d_indoor_detection',
            executable='mono3d_indoor_detection',
            output='screen',
            parameters=[
                {"config_file_path": "./config"},
                {"feed_image": "./config/images/3d_detection.png"},
                {"ai_msg_pub_topic_name": "ai_msg_3d_detection"},
                {"dump_render_img": 1}
            ],
            arguments=['--ros-args', '--log-level', 'info']
        )
    ])
