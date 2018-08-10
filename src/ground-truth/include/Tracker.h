/******************************************************************************
 *                                                                            *
 * Copyright (C) 2018 Fondazione Istituto Italiano di Tecnologia (IIT)        *
 * All Rights Reserved.                                                       *
 *                                                                            *
 ******************************************************************************/

/**
 * @authors: Nicola Piga
 */

#ifndef TRACKER_H
#define TRACKER_H

// yarp
#include <yarp/os/RFModule.h>
#include <yarp/sig/Image.h>
#include <yarp/dev/PolyDriver.h>
#include <yarp/dev/IFrameTransform.h>

// opencv
#include <opencv2/opencv.hpp>

//
#include <HeadKinematics.h>
#include <Tracker.h>
#include <UcoBoardEstimator.h>
#include <GazeController.h>
#include <KalmanFilter.h>

enum class Status { Idle, Hold, Track};

class Tracker : public yarp::os::RFModule
{
private:
    // head kinematics
    /* headKinematics head_kin; */

    // gaze controller
    GazeController gaze_ctrl;

    // aruco/charuco board estimator
    std::unique_ptr<UcoBoardEstimator> uco_estimator;

    // camera port
    yarp::os::BufferedPort<yarp::sig::ImageOf<yarp::sig::PixelRgb>> image_input_port;
    yarp::os::BufferedPort<yarp::sig::ImageOf<yarp::sig::PixelRgb>> image_output_port;

    // selected eye name
    std::string eye_name;

    // tracking source
    // i.e. whether to track the object with the eyes
    // using the estimate from the ground truth
    // or from the external filter
    std::string tracking_source;

    // period
    double period;

    // frame transform client
    yarp::dev::PolyDriver drv_transform_client;
    yarp::dev::IFrameTransform* tf_client;
    std::string tf_source;
    std::string tf_target;
    std::string sim_tf_source;
    std::string sim_tf_target;
    std::string filter_tf_source;
    std::string filter_tf_target;

    // kalman filter
    // required to filter aruco estimate
    KalmanFilter kf;

    // object sizes
    double obj_width;
    double obj_depth;
    double obj_height;

    // estimate
    yarp::sig::Vector est_pose;
    yarp::sig::Matrix est_pose_homog;
    yarp::sig::Vector initial_pose;
    bool is_estimate_available;
    bool use_kf;

    // estimate from external filter
    yarp::sig::Matrix filter_pose;
    bool is_filter_est_available;

    // status
    Status status;

    //  whether to publish images
    // with the superimposed estimated pose or not
    bool publish_images;

    // simulation mode
    // in simulation mode the tracker take the groun truth
    // from an external frame transform
    bool simulation_mode;

    // rpc server
    yarp::os::RpcServer rpc_port;
    yarp::os::Mutex mutex;

    bool getFrame(yarp::sig::ImageOf<yarp::sig::PixelRgb>* &yarp_image);
    bool retrieveGroundTruthSim(yarp::sig::Matrix &est_pose);
    bool retrieveExternalFilterEstimate(yarp::sig::Matrix &est_pose);
    bool evaluateEstimate(const cv::Mat &pos_wrt_cam, const cv::Mat &att_wrt_cam,
                          const yarp::sig::Vector &camera_pos,
                          const yarp::sig::Vector &camera_att,
                          yarp::sig::Vector &est_pose);
    void transformToCenter();
    void publishEstimate();
    yarp::sig::Vector homogToVector(const yarp::sig::Matrix &homog);
    yarp::sig::Matrix vectorToHomog(const yarp::sig::Vector &vector);
    yarp::sig::Matrix eulerZYX2dcm(const yarp::sig::Vector &euler);
    void initializeKF();
    void filterKF();
    void fixateWithEyes(const double &sync);
    void fixateWithEyesAndHold();
public:
    bool configure(yarp::os::ResourceFinder &rf) override;
    double getPeriod() override;
    bool updateModule() override;
    bool respond(const yarp::os::Bottle &command, yarp::os::Bottle &reply);
    bool close() override;
};

#endif
