#include <stdio.h>
#include <string.h>
#include "nc_adas_extract.h"

/* INI는 freespace=1. Day 1 로그로 확인한 뒤 필요하면 바꾸면 됨 */
#define ADAS_FREESPACE_VALUE  1

int adas_extract(const pp_result_buf *det,
                 const pp_result_buf *seg,
                 const pp_result_buf *lane,
                 int overlay_w,
                 int overlay_h,
                 AdasResult *out)
{
    static uint64_t frame_id = 0;
    int c, i;

    if (out == NULL) {
        return -1;
    }

    memset(out, 0, sizeof(*out));
    out->frame_id = frame_id++;
    out->width = overlay_w;
    out->height = overlay_h;
    out->freespace_value = ADAS_FREESPACE_VALUE;
    out->global_risk = RISK_SAFE;

    /* 1) 객체: 클래스별 배열 → objects[] 한 줄 */
    if (det != NULL) {
        const stCnnPostprocessingResults *cnn = &det->cnn_result;

        for (c = 0; c < MAX_CNN_CLASS_CNT; c++) {
            for (i = 0; i < cnn->class_objs[c].obj_cnt; i++) {
                const stObjInfo *src = &cnn->class_objs[c].objs[i];

                if (out->object_count >= ADAS_MAX_OBJECTS) {
                    break;
                }

                out->objects[out->object_count].class_id = cnn->class_objs[c].class_id;
                out->objects[out->object_count].confidence = src->prob;
                out->objects[out->object_count].x = src->bbox.x; /* 이미 화면 좌표, 좌상단 */
                out->objects[out->object_count].y = src->bbox.y;
                out->objects[out->object_count].w = src->bbox.w;
                out->objects[out->object_count].h = src->bbox.h;
                out->object_count++;
            }
        }
    }

    /* 2) freespace: 마스크를 새로 만들지 않고 포인터만 넘김 */
    if (seg != NULL && seg->cnn_result.seg != NULL) {
        out->freespace_mask = (const uint8_t *)seg->cnn_result.seg;
        out->mask_width  = (int)seg->seg_info.width;
        out->mask_height = (int)seg->seg_info.height;
    }

    /* 3) 차선: 점 있는 것만 복사 */
    if (lane != NULL) {
        const stCnnPostprocessingResults *cnn = &lane->cnn_result;

        for (i = 0; i < MAX_LANE_DET_CNT && out->lane_count < ADAS_MAX_LANES; i++) {
            const stLaneDet *src = &cnn->lane_det[i];
            int p, n;

            if (src->point_cnt <= 0) {
                continue;
            }

            n = src->point_cnt;
            if (n > ADAS_MAX_LANE_POINTS) {
                n = ADAS_MAX_LANE_POINTS;
            }

            out->lanes[out->lane_count].point_cnt = n;
            out->lanes[out->lane_count].lane_class = src->lane_class;
            for (p = 0; p < n; p++) {
                out->lanes[out->lane_count].points[p].x = (int)src->point[p].x;
                out->lanes[out->lane_count].points[p].y = (int)src->point[p].y;
            }
            out->lane_count++;
        }
    }

    return 0;
}

void adas_extract_debug_print(const AdasResult *r)
{
    int i;
    int cx, cy, mx = -1, my = -1;
    int mask_val = -1;

    if (r == NULL) {
        return;
    }

    printf("frame=%llu objects=%d lanes=%d mask=%dx%d\n",
           (unsigned long long)r->frame_id,
           r->object_count,
           r->lane_count,
           r->mask_width,
           r->mask_height);

    for (i = 0; i < r->object_count; i++) {
        const AdasObject *o = &r->objects[i];
        printf("  class=%d conf=%.2f xywh=(%.0f,%.0f,%.0f,%.0f)\n",
               o->class_id, o->confidence, o->x, o->y, o->w, o->h);
    }

    /* 첫 객체 바닥 중앙이 마스크에서 몇인지. B가 쓸 좌표 검증용 */
    if (r->object_count > 0 && r->freespace_mask != NULL &&
        r->mask_width > 0 && r->mask_height > 0 &&
        r->width > 0 && r->height > 0) {
        cx = (int)(r->objects[0].x + r->objects[0].w * 0.5f);
        cy = (int)(r->objects[0].y + r->objects[0].h);
        mx = cx * r->mask_width  / r->width;
        my = cy * r->mask_height / r->height;
        if (mx >= 0 && mx < r->mask_width && my >= 0 && my < r->mask_height) {
            mask_val = r->freespace_mask[my * r->mask_width + mx];
        }
        printf("  obj0 bottom=(%d,%d) -> mask(%d,%d)=%d\n",
               cx, cy, mx, my, mask_val);
    }
}