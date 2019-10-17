
#include "../inc/image_proc.h"

#include <stdio.h>  //printf
#include <stdlib.h> //malloc

void print_list_rect(list *Rects)
{
    int i = 0;
    list *current = Rects;
    while (current != NULL)
    {
        i = i + 1;
        current->id = i;
        printf("%d:[%d %d %d %d]\n", current->id, current->rect.x, current->rect.y, current->rect.width, current->rect.height);
        current = current->next;
    }
}

void push(list **Rects, CvRect rect)
{
    list *current = *Rects;
    if (current == NULL)
    {
        current = (list *)malloc(sizeof(list));
        current->rect = rect;

        current->id = 0;
        current->prev = NULL;
        current->next = NULL;
        current->level = 0;
        current->status = 0;
        current->angle = -1;
        *Rects = current;
    }
    else
    {
        while (current->next != NULL) 
        {
            current = current->next;
        }
        current->next = (list *)malloc(sizeof(list));
        current->next->id = current->id + 1;
        current->next->prev = current;
        current->next->next = NULL;
        current->next->rect = rect;
        current->next->level = 0;
        current->next->status = 0;
        current->next->angle = -1;
    }
}
int get_size(list *Rects)
{
    int i = 0;
    list *current = Rects;
    while (current != NULL)
    {
        i = i + 1;
        current->id = i;
        current = current->next;
    }
    return i;
}

int remove_by_index(list **Rects, int index)
{
    int i = 0;
    list *current = *Rects;
    list *temp_list = NULL;

    if (index == 0)
    {
        (*Rects) = current->next;
        if (*Rects != NULL)
            (*Rects)->prev = NULL;
        free(current);
        return 0;
    }
    for (i = 0; i < index - 1; i++)
    {
        if (current->next == NULL) return -1;
        current = current->next;
    }
    temp_list = current->next;
    current->next = temp_list->next;
    if (current->next != NULL) current->next->prev = temp_list->prev;

    free(temp_list);
    return 0;
}

void release(list **Rects)
{
    struct list *temp = *Rects;
    *Rects = (*Rects)->next;
    while ((*Rects) != NULL)
    {
        free(temp);
        temp = *Rects;
        (*Rects) = (*Rects)->next;
    }
}

CvRect rect_max(CvRect rect1, CvRect rect2)
{
    CvRect rect;
    rect.x = MIN(rect1.x, rect2.x);
    rect.y = MIN(rect1.y, rect2.y);
    rect.width = MAX((rect1.x + rect1.width), (rect2.x + rect2.width)) - rect.x;
    rect.height = MAX((rect1.y + rect1.height), (rect2.y + rect2.height)) - rect.y;
    return rect;
}

void connect_nearby_rects(list **Rects, int delta_w, int delta_h)
{
    int i = 0, j = 0, x1 = 0, x2 = 0, y1 = 0, y2 = 0, flag = 0;
    int N = get_size(*Rects);
    list *list_i = *Rects;
    list *list_j = *Rects;

    while (i < N)
    {
        while (j < N)
        {
            if (i != j)
            {
                x1 = list_i->rect.x - list_j->rect.width - delta_w;
                x2 = list_i->rect.x + list_i->rect.width + delta_w;
                y1 = list_i->rect.y - list_j->rect.height - delta_h;
                y2 = list_i->rect.y + list_i->rect.height + delta_h;
                if (list_j->rect.x >= x1 && list_j->rect.x <= x2 && list_j->rect.y >= y1 && list_j->rect.y <= y2)
                {
                    list_i->rect = rect_max(list_i->rect, list_j->rect);
                    remove_by_index(Rects, j);
                    N = get_size(*Rects);
                    flag = 1;
                    break;
                }
            }
            j++;
            list_j = list_j->next;
        }
        if (flag)
        {
            i = 0;
            j = 1;
            list_i = *Rects;
            list_j = (*Rects)->next;
            flag = 0;
        }
        else
        {
            j = 0;
            list_j = (*Rects);
            i++;
            list_i = list_i->next;
        }
    }
}

// get data stream
int data_stream(AVFormatContext **infoContext, char *devName)
{
    int ret = 0;
    AVFormatContext *pInfoContext = *infoContext;
    /* register all formats and codecs */
    av_log_set_level(56); //AV_LOG_DEBUG = 56
    av_register_all();
    avcodec_register_all();
    avformat_network_init();
    avdevice_register_all();

    /*open dev device*/
    //------------------------------------------------------------------------------
    //char *dev_name = "/dev/video53";
    //char *dev_name = "rtsp://10.2.9.141:43794/profile2";
    AVInputFormat *inputFormat = av_find_input_format("video4linux2"); // avformat.h
    if (!inputFormat)
    {
        av_log(0, AV_LOG_ERROR, "Cannot find input format\n");
    }

    AVDictionary *options = NULL;
    // check video source
    if (avformat_open_input(&pInfoContext, devName, inputFormat, &options) != 0)
    {
        printf("\nOops, could'nt open video source\n");
    }
    if ((ret = avformat_find_stream_info(pInfoContext, 0)) < 0)
    {
        fprintf(stderr, "Failed to retrieve input stream information");
    }
    snprintf(pInfoContext->filename, sizeof(pInfoContext->filename), "%s", devName);
    av_dump_format(pInfoContext, 0, devName, 0);

    *infoContext = pInfoContext;

    return ret;
}

void resize_image(IplImage *img, IplImage *img_resized)
{

    char *p = img_resized->imageData;
    int scale_w = img->width / img_resized->width;
    int scale_h = img->height / img_resized->height;
    int i = 0, j = 0;

    for (i = 0; i < img->height; i++)
    {
        if (i % scale_h == 0)
        {
            for (j = 0; j < img->width; j++)
            {
                if (j % scale_w == 0)
                {
                    *(p++) = *(img->imageData + i * img->width + j);
                }
            }
        }
    }
}

//create background
void create_background(IplImage *currentFrame, IplImage *background)
{
    uchar *bg = background->imageData;
    uchar *cf = currentFrame->imageData;
    int sizeImage = currentFrame->width * currentFrame->height;
    int i = 0;

    for (i = 0; i < sizeImage; i++, bg++, cf++)
    {
        if (*bg > *cf)
        {
            (*bg)--;
            continue;
        }
        if (*bg < *cf)
        {
            (*bg)++;
            continue;
        }
    }
}

//find foreground
void find_foreground(IplImage *grayImage, IplImage *grayBackground, IplImage *foreground, int valThresh, int learningRate)
{

    uchar *cf = grayImage->imageData;
    uchar *bg = grayBackground->imageData;
    uchar *fg = foreground->imageData;
    int i = 0;
    unsigned char absDiff = 0, maxThresh = 255;
    int sizeImage = grayImage->width * grayImage->height;

    if (learningRate == 0)
    {
        for (i = 0; i < sizeImage; i++, cf++, bg++, fg++)
        {
            // find foreground : background subtraction
            absDiff = abs((*cf) - (*bg));
            *fg = ((int)absDiff > valThresh) ? maxThresh : 0;

            //update background
            if (*bg > *cf)
            {
                (*bg)--;
                continue;
            }
            if (*bg < *cf)
            {
                (*bg)++;
                continue;
            }
        }
    }
    else
    {
        for (i = 0; i < sizeImage; i++, cf++, bg++, fg++)
        {
            // find foreground : background subtraction
            absDiff = abs((*cf) - (*bg));
            *fg = ((int)absDiff > valThresh) ? maxThresh : 0;
        }
    }
}

int motion_detect(IplImage *grayImage, IplImage *grayBackground, list **listTrack, int learningRate)
{
    list *Rects = NULL;
    CvRect rect;
    CvSeq *contours = NULL;
    CvMemStorage *storage = cvCreateMemStorage(0);
    IplImage *foreground = cvCreateImage(cvSize(grayImage->width, grayImage->height), grayImage->depth, 1);

    // foreground
    find_foreground(grayImage, grayBackground, foreground, 18, learningRate);

    // find contours
    cvFindContours(foreground, storage, &contours, sizeof(CvContour),
                   CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE, cvPoint(0, 0));

    for (; contours != NULL; contours = contours->h_next)
    {
        if (fabs(cvContourArea(contours, CV_WHOLE_SEQ)) >= 4)
        {
            rect = cvBoundingRect(contours, 0);
            push(&Rects, rect);
        }
    }

    cvReleaseMemStorage(&storage);
    cvReleaseImage(&foreground);

    int Num = get_size(Rects);
    if (Num)
    {
        // Connect nearby rectangles
        connect_nearby_rects(&Rects, 3, 5);
        // object tracking
        object_tracking(Rects, listTrack);
        //Num = get_size(*listTrack);
        //printf("%d\n", Num);
        release(&Rects);
        return 1;
    }

    return 0;
}

int find_object_direction(CvPoint p1, CvPoint p2)
{
    int w = p2.x - p1.x;
    int h = p1.y - p2.y;
    float tangent = 0;

    // 0-E, 1-NE, 2-N, 3-NW, 4-W, 5-SW, 6-S, 7-SE
    //tan(alpha) = h/w
    if (w == 0)
    {
        if (h > 0) return 2;
        if (h < 0) return 6;
    }
    else
    {
        if (h == 0 && w > 0) return 0;
        if (h == 0 && w < 0) return 4;

        tangent = (float)h / w;
        if (tangent >= 0.6 && tangent <= 1.7)
        {
            if (w > 0 || h > 0) return 1;
            if (w < 0 || h < 0) return 5;
        }
        if (tangent >= -1.7 && tangent <= -0.6)
        {
            if (w < 0 || h > 0) return 3;
            if (w > 0 || h < 0) return 7;
        }
        if (tangent < 0.6 || tangent > -0.6)
        {
            if (w > 0) return 0;
            if (w < 0) return 6;
        }
        if (tangent > 1.7 || tangent < -1.7)
        {
            if (h > 0) return 2;
            if (h < 0) return 4;
        }
    }
    return -1;
}

double overlap_box(CvRect box1, CvRect box2)
{
    double ratio = 0;
    int xbox = MAX(box1.x, box2.x);
    int ybox = MAX(box1.y, box2.y);
    int wbox = MIN(box1.x + box1.width, box2.x + box2.width) - xbox;
    int hbox = MIN(box1.y + box1.height, box2.y + box2.height) - ybox;
    if (wbox <= 0 || hbox <= 0) return 0;    
    else
    {
        int AreaBox = wbox * hbox;
        int AreaMin = MIN(box1.width * box1.height, box2.width * box2.height);
        ratio = (double)AreaBox / AreaMin;
    }
    return ratio;
}

int object_tracking(list *rects, list **listTrack)
{
    list *track = *listTrack;
    list *rects_current = rects;
    int size_rects = get_size(rects_current);
    int i = 0, count_track = 0, distance = 0, angle_track = -1;
    CvPoint track_old, track_new;    

    if (track == NULL)
    {
        for (rects_current = rects; rects_current != NULL; rects_current = rects_current->next)
        {
            push(listTrack, rects_current->rect);
        }
    }
    else
    {
        for (rects_current = rects; rects_current != NULL; rects_current = rects_current->next)
        {
            rects_current->id = 0;
        }
        for (track = *listTrack, i = 0; track != NULL; i++)
        {
            count_track = 0;

            for (rects_current = rects; rects_current != NULL; rects_current = rects_current->next)
            {
                /*			
				if (rects_current->rect.width*rects_current->rect.height < 2) {
					rects_current->id = -1;
					count_track++;
					continue;
				}*/
                track_old.x = track->rect.x + track->rect.width / 2;
                track_old.y = track->rect.y + track->rect.height / 2;
                track_new.x = rects_current->rect.x + rects_current->rect.width / 2;
                track_new.y = rects_current->rect.y + rects_current->rect.height / 2;
                angle_track = find_object_direction(track_old, track_new);
                // if overlap > 10% then update box
                if (rects_current->status == 0 && overlap_box(track->rect, rects_current->rect) > 0.1)
                {
                    rects_current->id = track->id;
                    rects_current->status = 1;

                    // detect direction
                    track->angle = angle_track >= 0 ? angle_track : track->angle;
                    track->rect = rects_current->rect;
                }
                // if trackBox and currentBox haven't overlap
                else
                {
                    distance = (track_old.x - track_new.x) * (track_old.x - track_new.x) + (track_old.y - track_new.y) * (track_old.y - track_new.y);
                    int temp_angle = track->angle - 1 < 0 ? 7 : track->angle - 1;
                    if (temp_angle == 7 && angle_track == 0) temp_angle = -1;

                    if (angle_track >= temp_angle && angle_track <= track->angle + 1)
                    {
                        distance -= (((track->rect.width) * (track->rect.width) + (rects_current->rect.width) * (rects_current->rect.width)) / 4);
                        if (distance <= 16 && rects_current->status == 0)
                        {
                            rects_current->id = track->id;
                            rects_current->status = 1;
                            track->angle = angle_track;
                            track->rect = rects_current->rect;
                        }
                        else count_track++;
                    }
                    else count_track++;
                }
            }
            if (count_track == size_rects)
            {
                track->level++;
                // if over 2 frame is not detected then delete
                if (track->level > 1)
                {
                    if (track->prev == NULL)
                    {
                        remove_by_index(listTrack, 0);
                        track = *listTrack;
                        i--;
                        continue;
                    }
                    else
                    {
                        track = (track->prev);
                        remove_by_index(listTrack, i);
                    }
                    i--;
                }
            }
            track = track->next;
        }
        for (rects_current = rects; rects_current != NULL; rects_current = rects_current->next)
        {
            if (rects_current->id == 0) push(listTrack, rects_current->rect);
        }
    }
    if (*listTrack != NULL) return 1;
    return 0;
}

//#include <sys/stat.h>
// check last modification of file
int check_modification_file(const char *path, time_t oldMTime)
{
    struct stat fileStat;
    int err = stat(path, &fileStat);
    if (err != 0)
    {
        perror(" [file_is_modified] stat");
        exit(errno);
    }
    return fileStat.st_mtime > oldMTime;
}

// line crossing
void direction_line_crossing(CvPoint *p1, CvPoint *p2)
{
    // set default direction
    if (((p1->x < p2->x) && (p1->y > p2->y)) || ((p1->x > p2->x) && (p1->y >= p2->y)) || (p1->y > p2->y))
    {
        // swap point_1 and point_2
        CvPoint temp = *p1;
        *p1 = *p2;
        *p2 = temp;
    }
}

void get_minmax_line(CvPoint *p1, CvPoint *p2, CvPoint *pminLine, CvPoint *pmaxLine)
{

    // pmin_line : (x; y)min = min(xp1, xp2 ; yp1, yp2)
    pminLine->x = MIN(p1->x, p2->x);
    pminLine->y = MIN(p1->y, p2->y);
    // pmax_line : (x; y)max = max(xp1, xp2 ; yp1, yp2)
    pmaxLine->x = MAX(p1->x, p2->x);
    pmaxLine->y = MAX(p1->y, p2->y);
    // x1 = x2 || y1 = y2
    if (pminLine->x == pmaxLine->x)
    {
        pminLine->x -= 5;
        pmaxLine->x += 5;
    }
    if (pminLine->y == pmaxLine->y)
    {
        pminLine->y -= 5;
        pmaxLine->y += 5;
    }
}

/*int setup_line_input(char* filename, CvPoint* p1, CvPoint* p2,  CvPoint* pminLine, CvPoint* pmaxLine, int options){
	// check last modification of file


	// read file
	

	// set default direction
	direction_line_crossing(p1, p2);

	// create area to detect line crossing
	get_minmax_line(p1, p2, pminLine, pmaxLine);

	return 1;

}*/

void line_crossing(CvPoint p1, CvPoint p2, CvPoint pminLine, CvPoint pmaxLine, list *listTrack, int options)
{
    list *track = listTrack;
    int tmp = 0;
    for (track = listTrack; track != NULL; track = track->next)
    {
        if ((track->rect.x + track->rect.width / 2 > pminLine.x &&
             track->rect.x + track->rect.width / 2 < pmaxLine.x) &&
            (track->rect.y + track->rect.height / 2 > pminLine.y &&
             track->rect.y + track->rect.height / 2 < pmaxLine.y))
        {
            tmp = (p2.y - p1.y) * (track->rect.x + track->rect.width / 2) - (p2.x - p1.x) * (track->rect.y + track->rect.height / 2) + p2.x * p1.y - p2.y * p1.x;
            tmp = (tmp > 0 ? 1 : -1);
            if (track->status == 0)
                track->status = tmp;
            else
            {
                if (track->status == -1 && tmp == 1)
                {
                    if (options >= 0)
                    {
                        printf("\n-----------> Line crossing A-> B\n");
                    }
                }
                else
                {
                    if (track->status == 1 && tmp == -1)
                    {
                        if (options <= 0)
                        {
                            printf("-----------> Line crossing      B-> A\n");
                        }
                    }
                }
            }
            track->status = tmp;
        }
    }
}

// intrusion
int point_polygon_test(int nvert, int *vertx, int *verty, int testx, int testy)
{
    int i, j, c = 0;
    for (i = 0, j = nvert - 1; i < nvert; j = i++)
    {
        if (((verty[i] > testy) != (verty[j] > testy)) &&
            (testx < (vertx[j] - vertx[i]) * (testy - verty[i]) / (verty[j] - verty[i]) + vertx[i]))
            c = !c;
    }
    return c;
}
int intrusion(list *rects, int nvert, int *vertx, int *verty)
{
    list *current = rects;
    int testx = 0, testy = 0;
    for (current = rects; current != NULL; current = current->next)
    {
        testx = current->rect.x + current->rect.width / 2;
        testy = current->rect.y + current->rect.height;
        if (point_polygon_test(nvert, vertx, verty, testx, testy))
        {
            return 1;
        }
    }
    return 0;
}