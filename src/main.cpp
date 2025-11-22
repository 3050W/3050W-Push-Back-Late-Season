
/*
██╗███╗   ██╗ ██████╗██╗     ██╗   ██╗██████╗ ███████╗
██║████╗  ██║██╔════╝██║     ██║   ██║██╔══██╗██╔════╝
██║██╔██╗ ██║██║     ██║     ██║   ██║██║  ██║█████╗  
██║██║╚██╗██║██║     ██║     ██║   ██║██║  ██║██╔══╝  
██║██║ ╚████║╚██████╗███████╗╚██████╔╝██████╔╝███████╗
╚═╝╚═╝  ╚═══╝ ╚═════╝╚══════╝ ╚═════╝ ╚═════╝ ╚══════╝
                                                      
*/
#include "main.h"
#include "lemlib/api.hpp" // IWYU pragma: keep
#include "lemlib/chassis/chassis.hpp"
//#include "pros/adi.hpp"
#include "pros/abstract_motor.hpp"
#include "pros/adi.hpp"
#include "pros/imu.hpp"
#include "pros/llemu.hpp"
#include "pros/misc.h"
#include "pros/motors.h"
#include "pros/optical.hpp"
#include "pros/rtos.hpp"
#include <cstdio>
#include <cstdlib>
#include <iostream>


/*
██████╗ ███████╗ ██████╗██╗      █████╗ ██████╗ ███████╗
██╔══██╗██╔════╝██╔════╝██║     ██╔══██╗██╔══██╗██╔════╝
██║  ██║█████╗  ██║     ██║     ███████║██████╔╝█████╗  
██║  ██║██╔══╝  ██║     ██║     ██╔══██║██╔══██╗██╔══╝  
██████╔╝███████╗╚██████╗███████╗██║  ██║██║  ██║███████╗
╚═════╝ ╚══════╝ ╚═════╝╚══════╝╚═╝  ╚═╝╚═╝  ╚═╝╚══════╝

*/

// controller
pros::Controller controller(pros::E_CONTROLLER_MASTER);

// motor groups
pros::MotorGroup leftMotors({-17, -18, -20},
                            pros::MotorGearset::blue); // left motor group - ports 3 (reversed), 4, 5 (reversed)
pros::MotorGroup rightMotors({16, 15, 14}, 
                            pros::MotorGearset::blue); // right motor group - ports 6, 7, 9 (reversed)


pros::Motor intake1(10, pros::MotorGearset::blue);
pros::Motor intake2(12, pros::MotorGearset::blue);

pros::Motor yMotor(9, pros::MotorGearset::blue);

// Inertial Sensor on port 10
pros::Imu imu(19);

// Optical Sensor on port 6
pros::Optical detector(13);
pros::ADIDigitalOut height('B');
pros::ADIDigitalOut doinker('C');
pros::ADIDigitalOut pod('D');
pros::ADIDigitalOut wing('A');
pros::ADIDigitalOut dblPark('E');
//pros::ADIButton selector('D');


/*
██████╗  █████╗ ███████╗██╗ ██████╗    ███████╗██╗  ██╗███╗   ██╗
██╔══██╗██╔══██╗██╔════╝██║██╔════╝    ██╔════╝╚██╗██╔╝████╗  ██║
██████╔╝███████║███████╗██║██║         █████╗   ╚███╔╝ ██╔██╗ ██║
██╔══██╗██╔══██║╚════██║██║██║         ██╔══╝   ██╔██╗ ██║╚██╗██║
██████╔╝██║  ██║███████║██║╚██████╗    ██║     ██╔╝ ██╗██║ ╚████║
╚═════╝ ╚═╝  ╚═╝╚══════╝╚═╝ ╚═════╝    ╚═╝     ╚═╝  ╚═╝╚═╝  ╚═══╝
                                                
*/

bool heightState = false;
bool doinkerState = false;
bool podState = false;
bool wingState = false;
bool parkState = false;

void toggleHeight() {
    heightState = !heightState;  // Toggle state
    height.set_value(heightState);
    pros::delay(10);
}

void toggleDoinker() {
    doinkerState = !doinkerState;  // Toggle state
    doinker.set_value(doinkerState);
    pros::delay(10);
}

void togglePod() {
    podState = !podState;
    pod.set_value(podState);
    pros::delay(10);
}

void toggleWings() {
    wingState = !wingState;
    wing.set_value(wingState);
    pros::delay(10);
}

void togglePark() {
    parkState = !parkState;
    dblPark.set_value(parkState);
    pros::delay(10);
}

void intaking(double speed) {
    intake1.move(speed);
    intake2.move(-speed*0.80);
}

void intakingStore(double speed) {
    intake1.move(speed);
    intake2.move(speed*0.5);
}

void intakeStop() {
    intake1.set_brake_mode(pros::E_MOTOR_BRAKE_HOLD);
    intake1.brake();
    intake2.set_brake_mode(pros::E_MOTOR_BRAKE_HOLD);
    intake2.brake();
}

/*
██████╗ ██████╗ ██╗██╗   ██╗███████╗    ██████╗ ███████╗ ██████╗██╗      █████╗ ██████╗ ███████╗
██╔══██╗██╔══██╗██║██║   ██║██╔════╝    ██╔══██╗██╔════╝██╔════╝██║     ██╔══██╗██╔══██╗██╔════╝
██║  ██║██████╔╝██║██║   ██║█████╗      ██║  ██║█████╗  ██║     ██║     ███████║██████╔╝█████╗  
██║  ██║██╔══██╗██║╚██╗ ██╔╝██╔══╝      ██║  ██║██╔══╝  ██║     ██║     ██╔══██║██╔══██╗██╔══╝  
██████╔╝██║  ██║██║ ╚████╔╝ ███████╗    ██████╔╝███████╗╚██████╗███████╗██║  ██║██║  ██║███████╗
╚═════╝ ╚═╝  ╚═╝╚═╝  ╚═══╝  ╚══════╝    ╚═════╝ ╚══════╝ ╚═════╝╚══════╝╚═╝  ╚═╝╚═╝  ╚═╝╚══════╝
                                                                                                
*/

// tracking wheels
// horizontal tracking wheel encoder. Rotation sensor, port 20, not reversed
pros::Rotation horizontalEnc(11);
// vertical tracking wheel encoder
//pros::Rotation verticalEnc(-11);
// horizontal tracking wheel. 2.75" diameter, 5.75" offset, back of the robot (negative)
lemlib::TrackingWheel horizontal(&horizontalEnc, lemlib::Omniwheel::NEW_275, 2.5);

// vertical tracking wheel. 2.75" diameter, 2.5" offset, left of the robot (negative)
//lemlib::TrackingWheel vertical(&verticalEnc, lemlib::Omniwheel::NEW_325, -5.5);


// drivetrain settings
lemlib::Drivetrain drivetrain(&leftMotors, // left motor group
                              &rightMotors, // right motor group
                              11.5, // 12 inch track width
                              lemlib::Omniwheel::NEW_325, // using new 4" omnis
                              450, // drivetrain rpm is 360
                              2 // horizontal drift is 2. If we had traction wheels, it would have been 8
);

// lateral motion controller
lemlib::ControllerSettings linearController(8, // proportional gain (kP)
                                            0, // integral gain (kI)
                                            27, // derivative gain (kD)
                                            3, // anti windup (3)
                                            0, // small error range, in inches (0.5)
                                            100, // small error range timeout, in milliseconds (100)
                                            0.3, // large error range, in inches (1)
                                            500, // large error range timeout, in milliseconds (500)
                                            0 // maximum acceleration (slew) (20)
);

// angular motion controller
lemlib::ControllerSettings angularController(4, // proportional gain (kP) 4
                                             0.2, // integral gain (kI) 0.02
                                             46, // derivative gain (kD) 46
                                             2, // anti windup (3)
                                             0, // small error range, in degrees (0.2)
                                             100, // small error range timeout, in milliseconds (100)
                                             0.5, // large error range, in degrees (0.7)
                                             500, // large error range timeout, in milliseconds (500)
                                             0 // maximum acceleration (slew) (20)
);

// sensors for odometry
lemlib::OdomSensors sensors(nullptr, // vertical tracking wheel
                            nullptr, // vertical tracking wheel 2, set to nullptr as we don't have a second one
                            &horizontal, // horizontal tracking wheel
                            nullptr, // horizontal tracking wheel 2, set to nullptr as we don't have a second one
                            &imu // inertial sensor
);

// input curve for throttle input during driver control
lemlib::ExpoDriveCurve throttleCurve(0, // joystick deadband out of 127
                                     5, // minimum output where drivetrain will move out of 127
                                     1.02 // expo curve gain
);

// input curve for steer input during driver control
lemlib::ExpoDriveCurve steerCurve(0, // joystick deadband out of 127
                                  5, // minimum output where drivetrain will move out of 127
                                  1.02 // expo curve gain
);

// create the chassis
lemlib::Chassis chassis(drivetrain, linearController, angularController, sensors, &throttleCurve, &steerCurve);

/*
 █████╗ ██╗   ██╗████████╗ ██████╗ ███╗   ██╗ ██████╗ ███╗   ███╗ ██████╗ ██╗   ██╗███████╗
██╔══██╗██║   ██║╚══██╔══╝██╔═══██╗████╗  ██║██╔═══██╗████╗ ████║██╔═══██╗██║   ██║██╔════╝
███████║██║   ██║   ██║   ██║   ██║██╔██╗ ██║██║   ██║██╔████╔██║██║   ██║██║   ██║███████╗
██╔══██║██║   ██║   ██║   ██║   ██║██║╚██╗██║██║   ██║██║╚██╔╝██║██║   ██║██║   ██║╚════██║
██║  ██║╚██████╔╝   ██║   ╚██████╔╝██║ ╚████║╚██████╔╝██║ ╚═╝ ██║╚██████╔╝╚██████╔╝███████║
╚═╝  ╚═╝ ╚═════╝    ╚═╝    ╚═════╝ ╚═╝  ╚═══╝ ╚═════╝ ╚═╝     ╚═╝ ╚═════╝  ╚═════╝ ╚══════╝
                                                                                           
*/

void midLow(double side) {

    chassis.moveToPoint(-10, 28, 500);
    pros::delay(200);
    intake1.move(117);
    chassis.moveToPoint(20, 37+5, 500, {.maxSpeed = 60});
    chassis.turnToHeading(80*side, 500);
    chassis.moveToPoint(33, 40+5, 700, {.maxSpeed = 20});
    chassis.turnToPoint(33, 50+5, 500);
    chassis.moveToPoint(33, 55+5, 500,{.maxSpeed = 20});
    chassis.moveToPoint(0, 42, 5000, {.forwards = false,.maxSpeed = 40});
    chassis.turnToPoint(-19, 69, 500);
    chassis.moveToPoint(-19,69,500,{.maxSpeed = 40});
    intake1.move(-95);
    pros::delay(2000);
    /*chassis.moveToPoint(0, 42, 5000,{.forwards = false,.maxSpeed = 40});
    chassis.moveToPoint(50, 54, 500);
    chassis.turnToHeading(90*side, 500);
    intake1.move(127);
    toggleHeight();
    chassis.moveToPoint(64, 60, 1000,{.maxSpeed = 50});
    pros::delay(1000);
    chassis.turnToHeading(90*side, 500);
    chassis.moveToPoint(15, 45, 1000,{.forwards = false,.maxSpeed = 50});
    chassis.turnToPoint(15, 25, 1000);
    chassis.moveToPoint(15, 25, 1000);
    toggleHeight();
    chassis.moveToPoint(28.5, 30, 1000);
    chassis.turnToHeading(5*side, 500);
    chassis.moveToPoint(30, 40, 1000);
    chassis.turnToHeading(5*side, 500);
    intaking(127);
    */
}

void midHigh(double side) {
    side = -1*side; 
    int shift = 5;
    chassis.moveToPoint(2-shift, 28, 500);
    pros::delay(200);
    toggleHeight();
    intake1.move(117);
    chassis.moveToPoint(-18-shift, 35, 500, {.maxSpeed = 60});
    chassis.turnToHeading(80*side, 500);
    chassis.moveToPoint(-20-shift, 40, 500, {.maxSpeed = 20});
    // deleted turnToPoint
    chassis.moveToPoint(0-shift, 42, 5000, {.forwards = false,.maxSpeed = 40});
    chassis.turnToPoint(22, 73, 500);
    chassis.moveToPoint(22,73,1000,{.maxSpeed = 40});
    pros::delay(500);
    intaking(127);
    pros::delay(2000);
    chassis.moveToPoint(0-shift, 45, 5000,{.forwards = false,.maxSpeed = 40});
    chassis.moveToPoint(-45-shift, 50, 500);
    chassis.turnToHeading(90*side, 500);
    intake2.set_brake_mode(pros::E_MOTOR_BRAKE_HOLD);
    intake2.brake();
    intake1.move(127);
    //toggleHeight();
    chassis.moveToPoint(-64-shift, 55, 1000,{.maxSpeed = 50});
    pros::delay(800);
    chassis.turnToHeading(90*side, 500);
    chassis.moveToPoint(-10-shift, 50, 1000,{.forwards = false,.maxSpeed = 50});
    chassis.turnToPoint(-10-shift, 25, 1000);
    chassis.moveToPoint(-10-shift, 25, 1000);
    toggleHeight();
    //chassis.turnToHeading(-90, 500);
    chassis.moveToPoint(-35, 25, 500);
    chassis.turnToHeading(0*side, 500);
    chassis.moveToPoint(-35, 40, 500);
    chassis.turnToHeading(0*side, 500);
    intaking(127);
}

void halfWPL() {
    chassis.moveToPoint(0, 10,1000);
    chassis.moveToPoint(6, 29, 1000,{.maxSpeed = 50});
    intaking(70);
    chassis.moveToPoint(0, 15, 1000, {.forwards = false});
    toggleDoinker();
    chassis.moveToPoint(0, 26, 1000,{.forwards = true});
    toggleDoinker();
    chassis.turnToHeading(-45, 1000);
    intaking(0);
    chassis.moveToPoint(-13, 47, 1000, {.maxSpeed = 50}); //15, 50
    pros::delay(250);
    intaking(-127);
    pros::delay(6700);
    chassis.moveToPoint(0, 20, 1000, {.forwards = false});
    /*chassis.moveToPoint(0, 20, 1000, {.forwards = false});
    chassis.turnToHeading(90, 100);
    chassis.turnToPoint(20, 17, 1000);
    chassis.moveToPoint(20, 17, 1000, {.maxSpeed = 90});
    chassis.turnToHeading(180, 1000);
    intaking(127);
    toggleDoinker();
    chassis.moveToPoint(22, -30, 1000, {.minSpeed = 40});
    pros::delay(2000);
    intaking(0);
    chassis.moveToPoint(29, -5, 1000, {.forwards=false});
    toggleDoinker();
    chassis.turnToHeading(0, 1000);
    chassis.moveToPoint(30, 20, 1000, {.maxSpeed=100});
    intaking(127);
    */
}

void nineBlocksM() {
    //chassis.moveToPoint(2, 12, 1000,{.maxSpeed = 80});
    chassis.moveToPoint(-7, 28, 1000,{.maxSpeed = 80});
    intaking(85);
    chassis.moveToPoint(-10, 30, 1000, {.maxSpeed = 60});
    toggleDoinker();
    pros::delay(250);
    
    chassis.moveToPoint(-15, 20, 1000,{.maxSpeed = 60});
    toggleDoinker();
    intaking(0);
    chassis.moveToPoint(-28, 14, 1000, {.maxSpeed = 80});
    //toggleDoinker();
    chassis.turnToHeading(0, 1000);
    chassis.moveToPoint(-33, 28, 1000,{.maxSpeed = 60});

    pros::delay(200);
    intaking(127);
    pros::delay(3000);

    chassis.moveToPoint(-33, 5, 1000,{.forwards = false});
    pros::delay(100);
    chassis.turnToHeading(180, 1000);

    
    intaking(127);

    toggleDoinker();
    chassis.moveToPoint(-33, -15, 1000,{.maxSpeed = 127, .minSpeed=100});
    chassis.turnToHeading(180, 1000);
    //chassis.moveToPoint(-33, -15, 1000,{.maxSpeed = 127, .minSpeed=100});
    pros::delay(1500);
    intaking(0);
    chassis.moveToPoint(-31, 7, 1000,{.forwards=false});
    chassis.turnToHeading(0, 1000);
    intaking(0);
    toggleDoinker();
    chassis.moveToPoint(-31, 25, 1000, {.maxSpeed=60});
    intaking(127);
}

void nineBlocksL() {
    intaking(85);
    chassis.moveToPoint(-5, 12, 1000,{.maxSpeed = 80});
    //chassis.turnToPoint(-6, 25, 1000);
    chassis.moveToPoint(7, 27, 1000,{.maxSpeed = 80});
    chassis.moveToPoint(9, 29, 1000, {.maxSpeed = 60});
    pros::delay(500);
    chassis.moveToPoint(11, 20, 1000,{.maxSpeed = 60});
    intaking(0);
    chassis.turnToHeading(90, 1000);
    chassis.moveToPoint(33, 14, 1000, {.maxSpeed = 80});
    chassis.turnToHeading(0, 1000);
    chassis.moveToPoint(35, 28, 1000,{.maxSpeed = 60});
    pros::delay(500);
    intaking(127);
    pros::delay(2000);
    chassis.moveToPoint(35, 15, 1000,{.forwards = false});
    chassis.turnToHeading(180, 1000);
    toggleDoinker();
    intaking(127);
    chassis.moveToPoint(35, -5, 1000,{.maxSpeed = 127, .minSpeed = 90});
    pros::delay(1000);
    chassis.moveToPoint(35, 0, 1000,{.forwards = false});
    chassis.turnToHeading(0, 1000);
    intaking(0);
    toggleDoinker();
    chassis.moveToPoint(35.5, 25, 1000,{.maxSpeed = 60});
    intaking(127);
}

void eightBlockM() {
    intakingStore(127);
    chassis.turnToPoint(2, 10, 100);
    chassis.moveToPoint(2, 10, 500);
    chassis.turnToPoint(-6, 29, 100); 
    chassis.moveToPoint(-6, 29, 1000, {.maxSpeed = 50});
    pros::delay(10);
    chassis.turnToPoint(-28, 55, 500); // 25, 46
    chassis.moveToPoint(-28, 55, 500);
    chassis.turnToHeading(-45, 1000);
    pros::delay(50);
    toggleDoinker();
    chassis.moveToPoint(-6, 28, 500, {.forwards = false});
    chassis.turnToHeading(0, 500);
    chassis.moveToPoint(-6, 15, 500, {.forwards = false});
    chassis.turnToPoint(-30, 15, 500);
    chassis.moveToPoint(-30, 15, 500);
    chassis.turnToHeading(180, 500);
    chassis.moveToPoint(-31, 31, 1000, {.forwards = false});
    intaking(-127);
    pros::delay(300);
    intaking(127);
    pros::delay(2000);
    intakingStore(127);
    chassis.turnToHeading(180, 500);
    chassis.turnToPoint(-31, -15, 100);
    chassis.moveToPoint(-31, 0, 500);
    chassis.moveToPoint(-31, -15, 1000, {.maxSpeed = 50});
    pros::delay(2000);
    chassis.turnToHeading(180, 500);
    chassis.moveToPoint(-30, 31, 1000, {.forwards = false, .maxSpeed = 80});
    pros::delay(500);
    intaking(-127);
    pros::delay(300);
    intaking(127);
    pros::delay(100);
    toggleDoinker();
}

void eightBlockL(){
    intakingStore(127);
    chassis.turnToPoint(-2, 10, 100);
    chassis.moveToPoint(-2, 10, 500);
    chassis.turnToPoint(6, 29, 100); 
    chassis.moveToPoint(6, 29, 1000, {.maxSpeed = 50});
    pros::delay(10);
    chassis.turnToPoint(18, 40, 500);
    chassis.moveToPoint(18, 40, 500);
    chassis.turnToHeading(75, 1000);
    pros::delay(50);
    toggleDoinker();
    chassis.moveToPoint(6, 28, 500, {.forwards = false});
    chassis.turnToHeading(0, 500);
    chassis.moveToPoint(6, 15, 500, {.forwards = false});
    chassis.turnToPoint(30, 15, 500);
    chassis.moveToPoint(30, 15, 500);
    chassis.turnToHeading(180, 500);
    chassis.moveToPoint(31, 31, 1000, {.forwards = false});
    intaking(-127);
    pros::delay(300);
    intaking(127);
    pros::delay(2000);
    intakingStore(127);
    chassis.turnToHeading(180, 500);
    chassis.turnToPoint(31, -15, 100);
    chassis.moveToPoint(31, 0, 500);
    chassis.moveToPoint(31, -15, 1000, {.maxSpeed = 50});
    pros::delay(2000);
    chassis.turnToHeading(180, 500);
    chassis.moveToPoint(31, 31, 1000, {.forwards = false});
    pros::delay(500);
    toggleDoinker();
    intaking(-127);
    pros::delay(300);
    intaking(127);
    pros::delay(100);
}

void skills(){
    chassis.setPose(0,0,0);
    chassis.moveToPoint(0, 10, 1000);
    chassis.turnToPoint(10, 10, 1000);
    chassis.moveToPoint(10, 10, 1000);
}

/*
██╗███╗   ██╗██╗████████╗██╗ █████╗ ██╗     ██╗███████╗███████╗
██║████╗  ██║██║╚══██╔══╝██║██╔══██╗██║     ██║╚══███╔╝██╔════╝
██║██╔██╗ ██║██║   ██║   ██║███████║██║     ██║  ███╔╝ █████╗  
██║██║╚██╗██║██║   ██║   ██║██╔══██║██║     ██║ ███╔╝  ██╔══╝  
██║██║ ╚████║██║   ██║   ██║██║  ██║███████╗██║███████╗███████╗
╚═╝╚═╝  ╚═══╝╚═╝   ╚═╝   ╚═╝╚═╝  ╚═╝╚══════╝╚═╝╚══════╝╚══════╝
                                                               
*/

void on_center_button() {
	static bool pressed = false;
	pressed = !pressed;
	if (pressed) {
		pros::lcd::set_text(2, "I was pressed!");
	} else {
		pros::lcd::clear_line(2);
	}
}

void initialize() {
	pros::lcd::initialize();
	chassis.calibrate();
}

/**
 * Runs while the robot is in the disabled state of Field Management System or
 * the VEX Competition Switch, following either autonomous or opcontrol. When
 * the robot is enabled, this task will exit.
 */
void disabled() {}

/**
 * Runs after initialize(), and before autonomous when connected to the Field
 * Management System or the VEX Competition Switch. This is intended for
 * competition-specific initialization routines, such as an autonomous selector
 * on the LCD.
 *
 * This task will exit when the robot is enabled and autonomous or opcontrol
 * starts.
 */
void competition_initialize() {}

/*
 █████╗ ██╗   ██╗████████╗ ██████╗ ███╗   ██╗ ██████╗ ███╗   ███╗ ██████╗ ██╗   ██╗███████╗
██╔══██╗██║   ██║╚══██╔══╝██╔═══██╗████╗  ██║██╔═══██╗████╗ ████║██╔═══██╗██║   ██║██╔════╝
███████║██║   ██║   ██║   ██║   ██║██╔██╗ ██║██║   ██║██╔████╔██║██║   ██║██║   ██║███████╗
██╔══██║██║   ██║   ██║   ██║   ██║██║╚██╗██║██║   ██║██║╚██╔╝██║██║   ██║██║   ██║╚════██║
██║  ██║╚██████╔╝   ██║   ╚██████╔╝██║ ╚████║╚██████╔╝██║ ╚═╝ ██║╚██████╔╝╚██████╔╝███████║
╚═╝  ╚═╝ ╚═════╝    ╚═╝    ╚═════╝ ╚═╝  ╚═══╝ ╚═════╝ ╚═╝     ╚═╝ ╚═════╝  ╚═════╝ ╚══════╝
                                                                                           
*/

/**
 * Runs the user autonomous code. This function will be started in its own task
 * with the default priority and stack size whenever the robot is enabled via
 * the Field Management System or the VEX Competition Switch in the autonomous
 * mode. Alternatively, this function may be called in initialize or opcontrol
 * for non-competition testing purposes.
 *
 * If the robot is disabled or communications is lost, the autonomous task
 * will be stopped. Re-enabling the robot will restart the task, not re-start it
 * from where it left off.
 */
void autonomous() {
    //midLow(1);
    //midHigh(1);
    //intaking(127);
    // 1 = red
    // -1 = blue

    // PID Tuning
    //chassis.turnToHeading(90, 1000);
    //chassis.moveToPoint(0, 12, 1000);
    
    //chassis.setBrakeMode(pros::E_MOTOR_BRAKE_BRAKE);
    skills();
    //eightBlockM();

}


/*
██████╗ ██████╗ ██╗██╗   ██╗███████╗██████╗ 
██╔══██╗██╔══██╗██║██║   ██║██╔════╝██╔══██╗
██║  ██║██████╔╝██║██║   ██║█████╗  ██████╔╝
██║  ██║██╔══██╗██║╚██╗ ██╔╝██╔══╝  ██╔══██╗
██████╔╝██║  ██║██║ ╚████╔╝ ███████╗██║  ██║
╚═════╝ ╚═╝  ╚═╝╚═╝  ╚═══╝  ╚══════╝╚═╝  ╚═╝
                                            
*/

/**
 * Runs the operator control code. This function will be started in its own task
 * with the default priority and stack size whenever the robot is enabled via
 * the Field Management System or the VEX Competition Switch in the operator
 * control mode.
 *
 * If no competition control is connected, this function will run immediately
 * following initialize().
 *
 * If the robot is disabled or communications is lost, the
 * operator control task will be stopped. Re-enabling the robot will restart the
 * task, not resume it from where it left off.
 */
void opcontrol() {
    togglePod();
    chassis.setBrakeMode(pros::E_MOTOR_BRAKE_COAST);
    
	while (true) {

		// get joystick positions (TANK)
        int leftY = controller.get_analog(pros::E_CONTROLLER_ANALOG_LEFT_Y);
        int rightY = controller.get_analog(pros::E_CONTROLLER_ANALOG_RIGHT_Y);

        // move the chassis with curvature drive
        chassis.tank(leftY, rightY);
        
        // print heading detected by IMU
        pros::lcd::print(6, "IMU: %f", imu.get_heading());

        // Intake Controls
        if (controller.get_digital(pros::E_CONTROLLER_DIGITAL_R2)) {
            // outtake top, intake bottom
            intake1.move(127*0.85);
            intake2.move(127*0.50);
        }
        else if (controller.get_digital(pros::E_CONTROLLER_DIGITAL_L2)) {
            // outtake both
            intake1.move(-127);
            intake2.move(127*0.80);
        }
        else if (controller.get_digital(pros::E_CONTROLLER_DIGITAL_R1)) {
            // intake both
            intake1.move(127*0.85);
            intake2.move(-127);
        }
        else{
            intake1.set_brake_mode(pros::E_MOTOR_BRAKE_HOLD);
            intake1.brake();
            intake2.set_brake_mode(pros::E_MOTOR_BRAKE_HOLD);
            intake2.brake();
        }

        if (controller.get_digital_new_press(pros::E_CONTROLLER_DIGITAL_Y)){
            toggleHeight();
        }

        if (controller.get_digital_new_press(pros::E_CONTROLLER_DIGITAL_RIGHT)){
            toggleDoinker();
        }

        if (controller.get_digital_new_press(pros::E_CONTROLLER_DIGITAL_DOWN)){
            toggleWings();
        }

        if (controller.get_digital_new_press(pros::E_CONTROLLER_DIGITAL_B)){
            togglePark();
        }

		pros::delay(20); 
	}
}