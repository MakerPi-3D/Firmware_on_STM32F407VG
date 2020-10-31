#include "gcodebufferhandle.h"
#include "Configuration.h"
#include "threed_engine.h"
#include "globalvariables_ccmram.h"
#include "sys_function.h"
#include "gcode_global_params.h"

#include <math.h>

namespace gcode
{
#define M_PI       3.14159
  static volatile uint32_t previous_millis_cmd = 0U;
  static volatile float offset[XYZ_NUM_AXIS] = {0.0F, 0.0F, 0.0F};

  extern void get_coordinates(void);

  extern void clamp_to_software_endstops(volatile float (&target)[MAX_NUM_AXIS]);

  void mc_arc(const volatile float (&position)[MAX_NUM_AXIS], const volatile float (&target)[MAX_NUM_AXIS], const volatile float (&offset)[3], uint8_t axis_0, uint8_t axis_1,
              uint8_t axis_linear, float feed_rate, float radius, uint8_t isclockwise, uint8_t extruder)
  {
    //   INT acceleration_manager_was_enabled = plan_is_acceleration_manager_enabled();
    //   plan_set_acceleration_manager_enabled(false); // disable acceleration management for the duration of the arc
    float center_axis0 = position[axis_0] + offset[axis_0];
    float center_axis1 = position[axis_1] + offset[axis_1]; // float
    float linear_travel = target[axis_linear] - position[axis_linear]; // float
    float extruder_travel = target[E_AXIS] - position[E_AXIS]; // float
    float r_axis0 = -offset[axis_0];  // Radius vector from center to current location
    float r_axis1 = -offset[axis_1];
    float rt_axis0 = target[axis_0] - center_axis0; // float
    float rt_axis1 = target[axis_1] - center_axis1; // float

    // CCW angle between position and target from circle center. Only one atan2() trig computation required.
    float angular_travel = atan2((r_axis0*rt_axis1)-(r_axis1*rt_axis0), (r_axis0*rt_axis0)+(r_axis1*rt_axis1)); // float
    if (angular_travel < 0)
    {
      angular_travel += 2*M_PI; // float
    }
    if (isclockwise)
    {
      angular_travel -= 2*M_PI; // float
    }

    float millimeters_of_travel = hypot(angular_travel*radius, fabs(linear_travel)); // float
    if (millimeters_of_travel < 0.001F)
    {
      return;
    }
    uint16_t segments = floor((millimeters_of_travel *100) /MM_PER_ARC_SEGMENT); // float
    if(segments == 0)
    {
      segments = 1;
    }

    /*
      // Multiply inverse feed_rate to compensate for the fact that this movement is approximated
      // by a number of discrete segments. The inverse feed_rate should be correct for the sum of
      // all segments.
      if (invert_feed_rate) { feed_rate *= segments; }
    */
    float theta_per_segment = angular_travel/segments/1.00F;
    float linear_per_segment = linear_travel/segments/1.00F; // float
    float extruder_per_segment = extruder_travel/segments/1.00F; // float

    /* Vector rotation by transformation matrix: r is the original vector, r_T is the rotated vector,
       and phi is the angle of rotation. Based on the solution approach by Jens Geisler.
           r_T = [cos(phi) -sin(phi);
                  sin(phi)  cos(phi] * r ;

       For arc generation, the center of the circle is the axis of rotation and the radius vector is
       defined from the circle center to the initial position. Each line segment is formed by successive
       vector rotations. This requires only two cos() and sin() computations to form the rotation
       matrix for the duration of the entire arc. Error may accumulate from numerical round-off, since
       all double numbers are single precision on the Arduino. (True double precision will not have
       round off issues for CNC applications.) Single precision error can accumulate to be greater than
       tool precision in some cases. Therefore, arc path correction is implemented.

       Small angle approximation may be used to reduce computation overhead further. This approximation
       holds for everything, but very small circles and large mm_per_arc_segment values. In other words,
       theta_per_segment would need to be greater than 0.1 rad and N_ARC_CORRECTION would need to be large
       to cause an appreciable drift error. N_ARC_CORRECTION~=25 is more than small enough to correct for
       numerical drift error. N_ARC_CORRECTION may be on the order a hundred(s) before error becomes an
       issue for CNC machines with the single precision Arduino calculations.

       This approximation also allows mc_arc to immediately insert a line segment into the planner
       without the initial overhead of computing cos() or sin(). By the time the arc needs to be applied
       a correction, the planner should have caught up to the lag caused by the initial mc_arc overhead.
       This is important when there are successive arc motions.
    */
    // Vector rotation matrix values
    float cos_T = 1-((0.5F*theta_per_segment)*theta_per_segment); // Small angle approximation
    float sin_T = theta_per_segment;

    float arc_target[MAX_NUM_AXIS];
    float sin_Ti;
    float cos_Ti;
    float r_axisi;
    uint16_t i;
    int8_t count = 0;

    // Initialize the linear axis
    arc_target[axis_linear] = position[axis_linear];

    // Initialize the extruder axis
    arc_target[E_AXIS] = position[E_AXIS];

    for (i = 1; i<segments; ++i)   // Increment (segments-1)
    {

      if (count < N_ARC_CORRECTION)
      {
        // Apply vector rotation matrix
        r_axisi = (r_axis0*sin_T) + (r_axis1*cos_T); // float
        r_axis0 = (r_axis0*cos_T) - (r_axis1*sin_T); // float
        r_axis1 = r_axisi;
        ++count;
      }
      else
      {
        // Arc correction to radius vector. Computed only every N_ARC_CORRECTION increments.
        // Compute exact location by applying transformation matrix from initial radius vector(=-offset).
        cos_Ti = cos(i*theta_per_segment);
        sin_Ti = sin(i*theta_per_segment); // float
        r_axis0 = (-offset[axis_0]*cos_Ti) + (offset[axis_1]*sin_Ti); // float
        r_axis1 = (-offset[axis_0]*sin_Ti) - (offset[axis_1]*cos_Ti); // float
        count = 0;
      }

      // Update arc_target location
      arc_target[axis_0] = center_axis0 + r_axis0;
      arc_target[axis_1] = center_axis1 + r_axis1; // float
      arc_target[axis_linear] += linear_per_segment; // float
      arc_target[E_AXIS] += extruder_per_segment; // float

      clamp_to_software_endstops(arc_target);
//    plan_buffer_line(arc_target,feed_rate, extruder);
      active_extruder = extruder;
      process_buffer_line_normal(arc_target,feed_rate);
    }
    // Ensure last segment arrives at target location.
//  plan_buffer_line(target,feed_rate, extruder);
    active_extruder = extruder;
    process_buffer_line_normal(arc_target,feed_rate);

    //   plan_set_acceleration_manager_enabled(acceleration_manager_was_enabled);
  }

  static void get_arc_coordinates(void)
  {
#ifdef SF_ARC_FIX
    bool relative_mode_backup = relative_mode;
    relative_mode = true;
#endif
    get_coordinates();
#ifdef SF_ARC_FIX
    relative_mode=relative_mode_backup;
#endif

    if(parseGcodeBufHandle.codeSeen('I'))
    {
      offset[0] = parseGcodeBufHandle.codeValue();
    }
    else
    {
      offset[0] = 0.0F;
    }
    if(parseGcodeBufHandle.codeSeen('J'))
    {
      offset[1] = parseGcodeBufHandle.codeValue();
    }
    else
    {
      offset[1] = 0.0F;
    }
  }

  static void prepare_arc_move(const volatile uint8_t isclockwise)
  {
    float r = hypot(offset[X_AXIS], offset[Y_AXIS]); // Compute arc radius for mc_arc

    // Trace the arc
    mc_arc(current_position, destination, offset, X_AXIS, Y_AXIS, Z_AXIS, ((feed_rate*static_cast<float>(feed_multiply))/60.0F)/100.0F, r, isclockwise, active_extruder);

    // As far as the parser is concerned, the position is now == target. In reality the
    // motion control system might still be processing the action and the real tool position
    // in any intermediate location.
    for(uint8_t i=0U; i < motion_3d.axis_num; ++i)
    {
      current_position[i] = destination[i];
    }
    previous_millis_cmd = sys_task_get_tick_count();
  }


  void g2_process(void)
  {
    get_arc_coordinates();
    prepare_arc_move(1);
  }

  void g3_process(void)
  {
    get_arc_coordinates();
    prepare_arc_move(0);
  }

}








