#include "diffdrive_arduino/diffdrive_arduino.h"

#include <exception>

#include "pluginlib/class_list_macros.hpp"

DiffDriveArduino::DiffDriveArduino()
: logger_(rclcpp::get_logger("DiffDriveArduino"))
{
}

hardware_interface::CallbackReturn DiffDriveArduino::on_init(
  const hardware_interface::HardwareInfo & info)
{
  if (hardware_interface::SystemInterface::on_init(info) !=
      hardware_interface::CallbackReturn::SUCCESS)
  {
    return hardware_interface::CallbackReturn::ERROR;
  }

  RCLCPP_INFO(logger_, "Configuring...");

  time_ = std::chrono::system_clock::now();

  cfg_.left_wheel_name = info_.hardware_parameters["left_wheel_name"];
  cfg_.right_wheel_name = info_.hardware_parameters["right_wheel_name"];
  cfg_.loop_rate = std::stof(info_.hardware_parameters["loop_rate"]);
  cfg_.device = info_.hardware_parameters["device"];
  cfg_.baud_rate = std::stoi(info_.hardware_parameters["baud_rate"]);
  cfg_.timeout = std::stoi(info_.hardware_parameters["timeout"]);
  cfg_.enc_counts_per_rev =
    std::stoi(info_.hardware_parameters["enc_counts_per_rev"]);

  l_wheel_.setup(cfg_.left_wheel_name, cfg_.enc_counts_per_rev);
  r_wheel_.setup(cfg_.right_wheel_name, cfg_.enc_counts_per_rev);

  arduino_.setup(cfg_.device, cfg_.baud_rate, cfg_.timeout);

  RCLCPP_INFO(logger_, "Finished Configuration");

  return hardware_interface::CallbackReturn::SUCCESS;
}

std::vector<hardware_interface::StateInterface>
DiffDriveArduino::export_state_interfaces()
{
  std::vector<hardware_interface::StateInterface> state_interfaces;

  state_interfaces.emplace_back(
    l_wheel_.name, hardware_interface::HW_IF_VELOCITY, &l_wheel_.vel);
  state_interfaces.emplace_back(
    l_wheel_.name, hardware_interface::HW_IF_POSITION, &l_wheel_.pos);
  state_interfaces.emplace_back(
    r_wheel_.name, hardware_interface::HW_IF_VELOCITY, &r_wheel_.vel);
  state_interfaces.emplace_back(
    r_wheel_.name, hardware_interface::HW_IF_POSITION, &r_wheel_.pos);

  return state_interfaces;
}

std::vector<hardware_interface::CommandInterface>
DiffDriveArduino::export_command_interfaces()
{
  std::vector<hardware_interface::CommandInterface> command_interfaces;

  command_interfaces.emplace_back(
    l_wheel_.name, hardware_interface::HW_IF_VELOCITY, &l_wheel_.cmd);
  command_interfaces.emplace_back(
    r_wheel_.name, hardware_interface::HW_IF_VELOCITY, &r_wheel_.cmd);

  return command_interfaces;
}

hardware_interface::CallbackReturn DiffDriveArduino::on_activate(
  const rclcpp_lifecycle::State & previous_state)
{
  (void)previous_state;

  RCLCPP_INFO(logger_, "Starting Controller...");

  arduino_.sendEmptyMsg();
  arduino_.setPidValues(30, 20, 0, 100);

  return hardware_interface::CallbackReturn::SUCCESS;
}

hardware_interface::CallbackReturn DiffDriveArduino::on_deactivate(
  const rclcpp_lifecycle::State & previous_state)
{
  (void)previous_state;

  RCLCPP_INFO(logger_, "Stopping Controller...");

  if (arduino_.connected()) {
    arduino_.setMotorValues(0, 0);
  }

  return hardware_interface::CallbackReturn::SUCCESS;
}

hardware_interface::return_type DiffDriveArduino::read(
  const rclcpp::Time & time,
  const rclcpp::Duration & period)
{
  (void)time;
  (void)period;

  auto new_time = std::chrono::system_clock::now();
  std::chrono::duration<double> diff = new_time - time_;
  double delta_seconds = diff.count();
  time_ = new_time;

  if (!arduino_.connected()) {
    return hardware_interface::return_type::ERROR;
  }

  try {
    arduino_.readEncoderValues(l_wheel_.enc, r_wheel_.enc);
  } catch (const std::exception & e) {
    RCLCPP_ERROR(logger_, "Failed to read encoder values: %s", e.what());
    return hardware_interface::return_type::ERROR;
  }

  if (delta_seconds <= 0.0) {
    return hardware_interface::return_type::OK;
  }

  double pos_prev = l_wheel_.pos;
  l_wheel_.pos = l_wheel_.calcEncAngle();
  l_wheel_.vel = (l_wheel_.pos - pos_prev) / delta_seconds;

  pos_prev = r_wheel_.pos;
  r_wheel_.pos = r_wheel_.calcEncAngle();
  r_wheel_.vel = (r_wheel_.pos - pos_prev) / delta_seconds;

  return hardware_interface::return_type::OK;
}

hardware_interface::return_type DiffDriveArduino::write(
  const rclcpp::Time & time,
  const rclcpp::Duration & period)
{
  (void)time;
  (void)period;

  if (!arduino_.connected()) {
    return hardware_interface::return_type::ERROR;
  }

  arduino_.setMotorValues(
    l_wheel_.cmd / l_wheel_.rads_per_count / cfg_.loop_rate,
    r_wheel_.cmd / r_wheel_.rads_per_count / cfg_.loop_rate);

  return hardware_interface::return_type::OK;
}

PLUGINLIB_EXPORT_CLASS(
  DiffDriveArduino,
  hardware_interface::SystemInterface
)