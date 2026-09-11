#define ADAS_MAX_OBJECTS 32
#define ADAS_MAX_LANE_POINTS 50
#define ADAS_MAX_LANES 10

typedef enum {
    RISK_SAFE = 0,
    RISK_WARNING,
    RISK_DANGER
} AdasRiskLevel;

typedef struct {
    int class_id;
    float confidence;
    float x, y, w, h;   /* SDK stBBox와 동일 */
} AdasObject;

typedef struct {
    int x, y;
} AdasPoint;

typedef struct {
    AdasPoint points[ADAS_MAX_LANE_POINTS];
    int point_cnt;
    int lane_class;
} AdasLane;

typedef struct {
    uint64_t frame_id;
    int width;          /* overlay width  */
    int height;         /* overlay height */

    AdasObject objects[ADAS_MAX_OBJECTS];
    int object_count;

    const uint8_t *freespace_mask;
    int mask_width;
    int mask_height;
    uint8_t freespace_value;   /* Day 1에 확인한 값 */

    AdasLane lanes[ADAS_MAX_LANES];
    int lane_count;

    AdasRiskLevel global_risk;
} AdasResult;