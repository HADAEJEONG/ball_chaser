#include "ros/ros.h"
#include "ball_chaser/DriveToTarget.h"
#include <sensor_msgs/Image.h>

// Define a global client that can request services
ros::ServiceClient client;

// This function calls the command_robot service to drive the robot in the specified direction
void drive_robot(float lin_x, float ang_z)
{
    // TODO: Request a service and pass the velocities to it to drive the robot
    // ROS_INFO("DriveToTargetRequest received - lin_x:%1.2f, ang_z:%1.2f", (float)lin_x, (float)ang_z);

    ball_chaser::DriveToTarget srv;
    srv.request.linear_x= lin_x;
    srv.request.angular_z= ang_z;

    client.call(srv);
}

// This callback function continuously executes and reads the image data
void process_image_callback(const sensor_msgs::Image img)
{

    int white_pixel = 255;
    float ang_z = 0;
    float lin_x = 0;
    float left_flag = 0;
    float right_flag =0;
    float center_flag = 0;
    float total_flag=0;
    
    for (int i = 0; i < img.height * img.step; i++) {
        if (i%img.width < img.width/3 && img.data[i] == white_pixel) {
            left_flag = left_flag +1; // left flag
        }
       
        else if (i%img.width >= (2*img.width)/3 && img.data[i] == white_pixel)  {
            right_flag = right_flag -1; // right flag
        }
     
        else if (img.width/3 <= i%img.width && i%img.width < (2*img.width)/3 && img.data[i] == white_pixel) {
            center_flag = center_flag +1;
        }
    }
    
    lin_x = 0.1;
    ang_z = 1.5707*(left_flag-right_flag)/(img.height * img.step); // angular velocity callibration
    total_flag = left_flag+right_flag+center_flag;

    ROS_INFO("total flag number:%1.2f", (float)total_flag);    


    if (total_flag > 30000){
           drive_robot(0, 0); 
    }  

    else{
           drive_robot(lin_x, ang_z);  // Go to the direction for the ball
    }
    // TODO: Loop through each pixel in the image and check if there's a bright white one
    // Then, identify if this pixel falls in the left, mid, or right side of the image
    // Depending on the white ball position, call the drive_bot function and pass velocities to it
    // Request a stop when there's no white ball seen by the camera
}

int main(int argc, char** argv)
{
    // Initialize the process_image node and create a handle to it
    ros::init(argc, argv, "process_image");
    ros::NodeHandle n;

    // Define a client service capable of requesting services from command_robot
    client = n.serviceClient<ball_chaser::DriveToTarget>("/ball_chaser/command_robot");

    // Subscribe to /camera/rgb/image_raw topic to read the image data inside the process_image_callback function
    ros::Subscriber sub1 = n.subscribe("/camera/rgb/image_raw", 10, process_image_callback);

    // Handle ROS communication events
    ros::spin();

    return 0;
}
