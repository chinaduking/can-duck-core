#include "libfcn/NetworkLayer.hpp"

#include "test_ServoDict.hpp"
#include "SimpleSerialNode.hpp"

using namespace libfcn_v2;
using namespace utils;


#define SERVO_ADDR 0x02


void runTest() {
	int local_addr = SERVO_ADDR;

	Tracer tracer(true);


	Node fcn_node(0);

	DataLinkFrame frame_tmp;

	auto rto_channel = fcn_node
			.network_layer->rto_network_handler.
			createChannel(fcnmsg::test_ServoPubSubDict, local_addr);

	auto rto_channel_2 = fcn_node
			.network_layer->rto_network_handler.
			createChannel(fcnmsg::test_ServoPubSubDict, local_addr);

	fcn_node.spin();

	uint32_t cnt = 0;

	for(int __i = 0; __i < 1; ){
		auto speed_msg = fcnmsg::test_ServoPubSubDict.speed;
		speed_msg << cnt;
		rto_channel->publish(speed_msg);

		auto angle_msg = fcnmsg::test_ServoPubSubDict.angle;
		angle_msg << cnt;
		rto_channel->publish(angle_msg);

		auto current_msg = fcnmsg::test_ServoPubSubDict.current;
		current_msg << cnt;
		rto_channel->publish(current_msg);

		frame_tmp.src_id = local_addr;
		frame_tmp.dest_id = 0x00; /*ANY*/

//            cout << Frame2Log(frame_tmp) << endl;
//
//            fcn_node.frame_dev->write(&frame_tmp);

		//fcn_node.spin();
		perciseSleep(0.1);

		LOGW("servo: speed = %d, angle = %d, current = %d \n",
					 rto_channel_2->readBuffer(fcnmsg::test_ServoPubSubDict.speed).data,
					 rto_channel_2->readBuffer(fcnmsg::test_ServoPubSubDict.angle).data,
					 rto_channel_2->readBuffer(fcnmsg::test_ServoPubSubDict.current).data
					 );

		cnt ++;
	}
}
