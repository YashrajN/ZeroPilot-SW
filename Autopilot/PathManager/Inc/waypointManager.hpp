/**
 * Waypoint Manager Methods and Helpers
 * Author: Dhruv Rawat
 * Created: November 2020
 * Last Updated: December 2020 (Dhruv)
 */

#ifndef WAYPOINT_MANAGER_H
#define WAYPOINT_MANAGER_H

#include <iostream>
#include <stdlib.h>
#include <math.h>

#define PATH_BUFFER_SIZE 100

struct _WaypointManager_Data_In {
    long double latitude;
    long double longitude;
    int altitude;
    uint16_t heading;
};

// Stores error codes for the waypoint manager
enum _WaypointStatus {WAYPOINT_SUCCESS = 0, WAYPOINT_UNDEFINED, INVALID_PARAMETER, UNDEFINED_FAILURE};

// Used in the waypointBufferStatus array to signal which elements are free
enum _WaypointBufferStatus {FREE = 0, FULL};

// Used to specify the modification type when updating the waypointBuffer array
enum _WaypointBufferUpdateType {APPEND_WAYPOINT = 0, UPDATE_WAYPOINT, INSERT_WAYPOINT, DELETE_WAYPOINT};

// Used to specify the type of output
enum _WaypointOutputType {PATH_FOLLOW = 0, ORBIT_FOLLOW, HOLD_WAYPOINT};

/**
* Structure stores information about the waypoints along our path to the destination and back.
*/

struct _PathData {
    int waypointId;                   // Id of the waypoint
    _PathData * next;                 // Next waypoint
    _PathData * previous;             // Previous waypoint
    long double latitude;             // Latitude of waypoint
    long double longitude;            // Longitude of waypoint
    int altitude;                     // Altitude of waypoint
    float turnRadius;                 // if hold is commanded (type = 2), then this is the radius of the hold cycle
    _WaypointOutputType waypointType; // 0 = regular waypoint, 2 = hold waypoint (plane will circle)
};

/**
* Structure contains the data that will be returned to the Path Manager state manager.
* This data will be used by the PID and coordinated turn engine to determine the commands to be sent to the Attitude Manager.
*/
struct _WaypointManager_Data_Out{
    uint16_t desiredHeading;            // Desired heading to stay on path
    int desiredAltitude;                // Desired altitude at next waypoint
    long double distanceToNextWaypoint; // Distance to the next waypoint (helps with airspeed PID)
    float radius;                       // Radius of turn if required
    int turnDirection;                  // Direction of turn -> -1 = CW (Right bank), 1 = CCW (Left bank). (Looking down from sky)
    _WaypointStatus errorCode;          // Contains error codes
    bool isDataNew;                     // Notifies PID modules if the data in this structure is new
    uint32_t timeOfData;                // The time that the data in this structure was collected
    _WaypointOutputType out_type;       // Output type (determines which parameters are defined)
};

class WaypointManager {
public:

    /**
    * Constructor for this class
    *
    * @param[in] float relLat -> This is the relative latitude of the point that will be used as (0,0) when converting lat-long coordinates to cartesian coordiantes. 
    * @param[in] float relLong -> This is the relative longitude of the point that will be used as (0,0) when converting lat-long coordinates to cartesian coordiantes.
    */
    WaypointManager(float relLat, float relLong); // Call this to get an instance of the class

    /**
    * Initializes the flight path
    *
    * @param[in] _PathData * initialWaypoints -> These waypoints will be used to initialize the waypointBuffer array
    * @param[in] int numberOfWaypoints -> Number of waypoints being initialized in the waypointBuffer array
    * @param[in] _PathData* currentLocation -> Home base.
    */
    _WaypointStatus initialize_flight_path(_PathData ** initialWaypoints, int numberOfWaypoints, _PathData *currentLocation); // Sets flight path and home base
    _WaypointStatus initialize_flight_path(_PathData ** initialWaypoints, int numberOfWaypoints);                             // Sets flight path

    /**
    * Updates the _WaypointManager_Data_Out structure with new values.
    *
    * @param[in] _Waypoint_Data_In currentPosition -> contains the current coordinates, altitude, and heading
    * @param[out] _WaypointManager_Data_Out &Data -> Memory address for a structure that holds the data for the state machine
    */
    _WaypointStatus get_next_directions(_WaypointManager_Data_In currentStatus, _WaypointManager_Data_Out *Data);

    /**
    * Adds, inserts, updates, or deletes a single waypoint in the waypointBuffer array
    *
    * @param[in] _PathData* waypoint -> In the instance that we need to update, insert, or append a new waypoint, this will be used
    * @param[in] _WaypointBufferUpdateType updateType -> the type of modification to the waypointBuffer array (look above)
    * @param[in] numWaypoints -> number of waypoints that are in the waypoint array (will be 1 for insertion, updating, and deleting). May be greater than 1 for appending
    * @param[in] int waypointId -> the ID of the waypoint that will be updated or deleted. Set to 0 by default, so does not need to be passed (Set to 0 when appending)
    * @param[in] int previousId -> stores the ID of the waypoint that will come before the inserted waypoint. Set to 0 by default, so does not need to be passed (Set to 0 when NOT inserting)
    * @param[in] int nextId -> stores the ID of the waypoint that will come after the inserted waypoint. Set to 0 by default, so does not need to be passed (Set to 0 when NOT inserting)
    */
    _WaypointStatus update_path_nodes(_PathData *waypoint, _WaypointBufferUpdateType updateType, int waypointId, int previousId, int nextId);

    /**
    * Clears the waypointBuffer array.
    */
    void clear_path_nodes();

    /**
    * @return the waypointBuffer array if requested
    */
    _PathData** get_waypoint_buffer();

    /**
    * @return the value of the waypointBufferStatus array at a specified index.
    * This can be used by the state machine to check if the waypointBuffer array is initialized at a specific index when iterating over it :))
    */
    _WaypointBufferStatus get_status_of_index(int index);

    /**
     * @return returns the _PathData pointer of the home base 
     */ 
    _PathData * get_home_base();

    /**
     * Called if user wants the plane to start circling
     *
     * Even while circling, state machine should call get_next_direction().
     * When user wants to exit this cycle, user can call this method again and pass in true for cancelTurning. This will set inHold to false.
     *
     * @param[in] _WaypointManager_Data_in currentStatus -> stores current gps info 
     * @param[in] float radius -> radius of the turn
     * @param[in] int direction -> -1 means clockwise (bank right); 1 means counter-clock wise (bank left)
     * @param[in] int altitude -> altitude of hold pattern
     * @param[in] bool cancelTurning -> false means we want plane to orbit. True means we want plane to stop orbiting and follow waypointBuffer array
     */
    void start_circling(_WaypointManager_Data_In currentStatus, float radius, int direction, int altitude, bool cancelTurning);

    /**
     * Called if user wants the plane to just head home
     *
     * Steps:
     *  - The method will clear the waypointBuffer array
     *  - Since gps coordiantes of the home base are already stored in the homeBase parameter, the plane will just fly towards that direction
     *  - As the plane flies, the state machine can call update_path_nodes() to update the waypointBuffer array. The state machine should also keep calling get_next_directions() to get desired heading and altitude
     *  - Once the waypoints are set, user can call this method again to make the plane follow the waypointBuffer array. (when calling this the second time, goingHome will be set to False)
     */
    void head_home();

    /**
     * Called by state machine to create new _PathData objects. This moves all heap allocation and ID management to the waypoint manager, giving the state machine less work
     *
     * First method returns an empty structure with only the ID initialized
     * Second method initializes a regular waypoint
     * Third method initializes a "hold" waypoint
     *
     * Parameters have the same name as their corresponding parameters in the _Pathdata struct.
     */
    _PathData* initialize_waypoint();                                                                                              // Creates a blank waypoint
    _PathData* initialize_waypoint(long double longitude, long double latitude, int altitude, _WaypointOutputType waypointType);                   // Initialize a regular waypoint
    _PathData* initialize_waypoint(long double longitude, long double latitude, int altitude, _WaypointOutputType waypointType, float turnRadius); // Initialize a "hold" waypoint

    // For testing purposes only:
    float orbitCentreLat;
    float orbitCentreLong;
    float orbitCentreAlt;

private:
    //Stores waypoints
    _PathData * waypointBuffer[PATH_BUFFER_SIZE]; //Stores all waypoints
    _WaypointBufferStatus waypointBufferStatus[PATH_BUFFER_SIZE] = {FREE}; //Keeps status of elements in waypointBuffer array
    int numWaypoints;
    int nextFilledIndex; // Index of the array that will be initialized next
    int nextAssignedId; // ID of the next waypoint that will be initialized
    int currentIndex;   // Index for the waypoint in our flight path we are currently on (If we are going from waypint A and B, currentIndex is the index of waypoint A)
    int orbitPathStatus; // Are we orbiting or following a straight path

    //Home base
    _PathData * homeBase;

    // Relative lat and long for coordinate calcilation
    float relativeLongitude;
    float relativeLatitude;

    //Data that will be transferred
    uint16_t desiredHeading;
    int desiredAltitude;
    long double distanceToNextWaypoint;
    _WaypointStatus errorCode;
    bool dataIsNew;
    _WaypointOutputType outputType;

    //Status variables
    bool goingHome;     // This is set to true when the head_home() function is called.
    _WaypointStatus errorStatus;
    bool inHold; // Set to true when start_circling() is called
    float turnCenter[3];
    int turnDesiredAltitude;
    int turnDirection; // 1 for CW, 2 for CCW
    float turnRadius;

    //Helper Methods
    void follow_hold_pattern(float* position, float heading);
    void follow_waypoints(_PathData * currentWaypoint, float* position, float heading);                               // Determines which of the methods below to call :))
    void follow_line_segment(_PathData * currentWaypoint, float* position, float heading);                            // In the instance where the waypoint after the next is not defined, we continue on the path we are currently on
    void follow_last_line_segment(_PathData * currentWaypoint, float* position, float heading);                       // In the instance where the next waypoint is not defined, follow previously defined path
    void follow_orbit(float* position, float heading);                                                                // Makes the plane follow an orbit with defined radius and direction
    void follow_straight_path(float* waypointDirection, float* targetWaypoint, float* position, float heading);       // Makes a plane follow a straight path (straight line following)
    float maintain_altitude(_PathData* currentPath);                                                                  // Makes plane maintain altitude

    void update_return_data(_WaypointManager_Data_Out *Data);       // Updates data in the output structure

    /**
    * Takes GPS long and lat data and converts it into coordinates (better for calculating headings and stuff)
    *
    * @param[in] long double longitude -> GPS longitide
    * @param[in] long double latitude -> GPS latitude
    * @param[out] float* xyCoordinates -> Array that will store the x and y coordinates of the plane
    */
    void get_coordinates(long double longitude, long double latitude, float* xyCoordinates);

    /**
     * Takes in two points and returns distance in metres
     *
     * @param[in] lat1, long2 --> Point 1
     * @param[in] lat2, long2 --> Point 2
     */
    float get_distance(long double lat1, long double lon1, long double lat2, long double lon2);

    /**
     * Removes waypoint from the heap
     */
    int destroy_waypoint(_PathData * waypoint);

    int get_waypoint_index_from_id(int waypointId);                                              // If provided a waypoint id, this method finds the element index in the waypointBuffer array

    _WaypointStatus append_waypoint(_PathData* newWaypoint);                                     // Adds a waypoint to the first free element in the waypointBuffer (array)
    _WaypointStatus insert_new_waypoint(_PathData* newWaypoint, int previousId, int nextId);     // Inserts new waypoint in between the specified waypoints (identified using the waypoint IDs). Note, you cannot insert a waypoint to waypointBuffer[0] or waypointBuffer[PATH_BUFFER_SIZE-1]
    _WaypointStatus delete_waypoint(int waypointId);                                             // Deletes the waypoint with the specified ID
    _WaypointStatus update_waypoint(_PathData* updatedWaypoint, int waypointId);                 // Updates the waypoint with the specified ID
};

#endif
