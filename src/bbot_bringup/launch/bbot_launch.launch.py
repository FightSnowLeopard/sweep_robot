import os
from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch.actions import IncludeLaunchDescription,TimerAction
from launch.launch_description_sources import PythonLaunchDescriptionSource
from launch_ros.actions import Node
from launch.actions import RegisterEventHandler
from launch.event_handlers import OnProcessStart

def generate_launch_description():
    package_name='bbot_description'
    slam_toolbox_name='slam_toolbox'

    bbot = IncludeLaunchDescription(
                PythonLaunchDescriptionSource([os.path.join(
                    get_package_share_directory(package_name), 'launch', 'bbot.launch.py'    
                    )]), launch_arguments={'use_sim_time': 'false'}.items()
            )
    # slam_toolbox = IncludeLaunchDescription(
    #             PythonLaunchDescriptionSource([os.path.join(
    #                 get_package_share_directory(slam_toolbox_name), 'launch', 'online_async_launch.py'    
    #                 )]), launch_arguments={'use_sim_time': 'true','params_file': '/home/ros/bbot_demo/src/bbot_bringup/config/mapper_params_online_async.yaml'}.items()
    #         )



    gazebo = IncludeLaunchDescription(
                PythonLaunchDescriptionSource([os.path.join(
                    get_package_share_directory('gazebo_ros'), 'launch', 'gazebo.launch.py'
                )]),
            )

    spawn_entity = Node(
        package='gazebo_ros', 
        executable='spawn_entity.py',
        arguments=['-topic', 'robot_description', '-entity', 'bbot'],
        output='screen'
    )
    delay_spawn_entity = TimerAction(period= 6.0,actions=[spawn_entity])

    joint_state = Node(
        package='controller_manager', 
        executable='spawner',
        arguments=['joint_state'],
    )

    differ_drive = Node(
        package='controller_manager', 
        executable='spawner',
        arguments=['differ_drive'],
    )

    trigger_spawners1 = RegisterEventHandler(
        event_handler=OnProcessStart(
            target_action=spawn_entity,
            on_start=[differ_drive]
        )
    )
    trigger_spawners2 = RegisterEventHandler(
        event_handler=OnProcessStart(
            target_action=spawn_entity,
            on_start=[joint_state]
        )
    )

    return LaunchDescription([
        bbot,
        gazebo,
        # slam_toolbox,
        delay_spawn_entity,
        trigger_spawners1,
        trigger_spawners2
    ])
