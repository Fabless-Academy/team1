#ifndef NC_ADAS_TYPES_H
#define NC_ADAS_TYPES_H

#include <stdint.h>


#define ADAS_MAX_OBJECTS      32
#define ADAS_MAX_LANE_POINTS 50
#define ADAS_MAX_LANES       10


typedef enum {
    RISK_SAFE = 0,
    RISK_WARNING,
    RISK_DANGER
} AdasRiskLevel;


typedef struct {
    int class_id;
    float confidence;

    float x;
    float y;
    float w;
    float h;

} AdasObject;


typedef struct {
    int x;
    int y;

} AdasPoint;


typedef struct {
    AdasPoint points[ADAS_MAX_LANE_POINTS];

    int point_cnt;
    int lane_class;

} AdasLane;


typedef struct {
    uint64_t frame_id;

    int width;
    int height;


    /*
     * Detection
     */
    AdasObject objects[ADAS_MAX_OBJECTS];
    int object_count;


    /*
     * Segmentation / Freespace
     */
    const uint8_t *freespace_mask;

    int mask_width;
    int mask_height;

    uint8_t freespace_value;


    /*
     * Lane
     */
    AdasLane lanes[ADAS_MAX_LANES];
    int lane_count;


    /*
     * Risk
     */
    AdasRiskLevel global_risk;

} AdasResult;


#endif /* NC_ADAS_TYPES_H */