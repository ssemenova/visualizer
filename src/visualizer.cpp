#include <functional>
#include <memory>

#include "opencv2/opencv.hpp"
#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/string.hpp"
#include "geometry_msgs/msg/pose.hpp"

using std::placeholders::_1;

class MinimalSubscriber : public rclcpp::Node
{
public:
  MinimalSubscriber()
  : Node("visualizer")
  {
    subscription_ = this->create_subscription<geometry_msgs::msg::Pose>(
      "pose", 10, std::bind(&MinimalSubscriber::topic_callback, this, _1));
     
    numFrames = 640;
    numKeyFrames = 10;

    cv::glob("/home/rusty/rgbd_dataset_freiburg1_desk2/rgb/*.png", frame_paths, false);
    
    if (!numFrames || frame_paths.size() == 0) {
			RCLCPP_INFO(this->get_logger(), "Error grabbing frame paths");
    }
			
    groundTruth = loadGroundTruth("/home/rusty/rgbd_dataset_freiburg1_desk2/groundtruth.txt");
    
    // Trajectory output
		traj = cv::Mat::zeros(750, 750, CV_8UC3);

		int numCallbackCalled = 0;

		RCLCPP_INFO(this->get_logger(), "Init done");
  }

private:
  // Defaults
  int numFrames;
  int numKeyFrames;
  std::vector<cv::String> frame_paths;
  std::string dataPath;
  int keyFrameNum;
  
  cv::Mat traj_t;
  cv::Mat traj_R;
  cv::Mat traj;
  
  // Viewer
  int numCallbackCalled;
  cv::Mat curr_frame;
  cv::Mat last_frame;
  std::vector<cv::Mat> groundTruth;
  
  void topic_callback(const geometry_msgs::msg::Pose::SharedPtr msg)
  {
    RCLCPP_INFO(this->get_logger(), "I heard: '%s'", msg->position.x);
    if (numCallbackCalled == 0) {
			last_frame = cv::imread(frame_paths[numCallbackCalled]);
			cv::cvtColor(last_frame, last_frame, cv::COLOR_BGR2GRAY);
    }
    
		curr_frame = cv::imread(frame_paths[numCallbackCalled]);
		cv::cvtColor(curr_frame, curr_frame, cv::COLOR_BGR2GRAY);
	
		cv::imshow("Road facing camera", curr_frame);

    double scale = 1; //getAbsoluteScale(numCallbackCalled, 0, t.at<double>(2));
    
    cv::Mat R = cv::Mat(1, 3, CV_32F, {msg->position.x, msg->position.y, msg->position.z});
   	float x = msg->position.x;
   	float y = msg->position.y;
   	float z = msg->position.z;

    //traj_t = traj_t + scale*(traj_R*t);
    //traj_R = traj_R;

		// VIEWER
		cv::Mat gt = groundTruth[numCallbackCalled];
		int x_truth = int(gt.at<double>(0,3));
		int z_truth = -int(gt.at<double>(2,3));

		//int x = int(traj_t.at<double>(0));
		//int z = -int(traj_t.at<double>(2));

		// Offsets to move trajectory towards the middle of viewer
		int x_offset = 375;
		int z_offset = 600;

		// Our calculated traj (red)
		cv::circle(traj, cv::Point(x+x_offset,z+z_offset), 1, CV_RGB(255,0,0), 2);
		// Ground truth (green)
		cv::circle(traj, cv::Point(x_truth+x_offset,z_truth+z_offset), 1, CV_RGB(0,255,0), 2);

		cv::imshow("Trajectory", traj );
	  cv::waitKey(1);
		  
		last_frame = curr_frame;
		numCallbackCalled++;
  }
  rclcpp::Subscription<geometry_msgs::msg::Pose>::SharedPtr subscription_;
  
  std::vector<cv::Mat> loadGroundTruth(std::string filename) 
  {
    std::vector<cv::Mat> gt;
    std::ifstream file(filename);
    std::string line;

    while (getline(file, line)) {
      cv::Mat temp = cv::Mat::zeros(3, 4, CV_64FC1);
      // cout << line << endl;
      sscanf(line.c_str(), "%lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf",
          &temp.at<double>(0,0), &temp.at<double>(0,1), &temp.at<double>(0,2), &temp.at<double>(0,3),
          &temp.at<double>(1,0), &temp.at<double>(1,1), &temp.at<double>(1,2), &temp.at<double>(1,3),
          &temp.at<double>(2,0), &temp.at<double>(2,1), &temp.at<double>(2,2), &temp.at<double>(2,3));

      gt.push_back(temp);
    }

    return gt;
	}
};

int main(int argc, char * argv[])
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<MinimalSubscriber>());
  rclcpp::shutdown();
  return 0;
}
