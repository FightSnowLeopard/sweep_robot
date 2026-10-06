import os

from ament_index_python.packages import get_package_share_directory

from launch import LaunchDescription
from launch.actions import IncludeLaunchDescription, TimerAction
from launch.launch_description_sources import PythonLaunchDescriptionSource
from launch.substitutions import Command, FindExecutable
from launch_ros.actions import Node


def generate_launch_description():

    description_package = get_package_share_directory(
        'bbot_description'
    )

    bringup_package = get_package_share_directory(
        'bbot_bringup'
    )

    # 机器人模型启动文件
    description_launch_file = os.path.join(
        description_package,
        'launch',
        'bbot.launch.py'
    )

    bbot_description_launch = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(
            description_launch_file
        )
    )

    # 生成 robot_description
    xacro_file = os.path.join(
        description_package,
        'urdf',
        'bbot.urdf.xacro'
    )

    robot_description = {
        'robot_description': Command([
            FindExecutable(name='xacro'),
            ' ',
            xacro_file
        ])
    }

    # ros2_control 控制器配置文件
    controller_config = os.path.join(
        bringup_package,
        'config',
        'bbot_controllers.yaml'
    )

    # 启动 ros2_control_node
    controller_manager = Node(
        package='controller_manager',
        executable='ros2_control_node',
        parameters=[
            robot_description,
            controller_config
        ],
        output='screen'
    )

    # 启动 joint_state 控制器
    joint_state_spawner = Node(
        package='controller_manager',
        executable='spawner',
        arguments=['joint_state'],
        output='screen'
    )

    # 启动差速控制器
    diff_drive_spawner = Node(
        package='controller_manager',
        executable='spawner',
        arguments=['diff_drive'],
        output='screen'
    )

    # 延迟启动 controller_manager
    delayed_controller_manager = TimerAction(
        period=3.0,
        actions=[
            controller_manager
        ]
    )

    # 延迟启动 joint_state
    delayed_joint_state_spawner = TimerAction(
        period=8.0,
        actions=[
            joint_state_spawner
        ]
    )

    # 延迟启动 diff_drive
    delayed_diff_drive_spawner = TimerAction(
        period=10.0,
        actions=[
            diff_drive_spawner
        ]
    )

    return LaunchDescription([
        # 启动 robot_state_publisher 和 RViz
        bbot_description_launch,

        # 启动 ros2_control
        delayed_controller_manager,

        # 启动两个控制器
        delayed_joint_state_spawner,
        delayed_diff_drive_spawner
    ])