#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
// Tên các file output và file log
const int max_so_dong = 10000;
const char ten_file_dau_vao[] = "data_filename.csv";
const char ten_file_vi_tri[] = "location.csv";
const char file_gia_tri_hop_le[] = "lux_valid.csv";
const char file_gia_tri_ngoai_le[] = "lux_outlier.csv";
const char file_dieu_kien[] = "lux_condition.csv";
const char file_tong_ket[] = "lux_summary.csv";
const char file_ghi_loi[] = "task2.log";
// Struct để lưu trữ dữ liệu hợp lệ (valid)
struct GiaTriHopLe {
    int id;
    char thoi_gian[20];
    float lux;
};
GiaTriHopLe du_lieu_hop_le[10000];
int so_dong_hop_le = 0;
// Các mức sáng theo từng khu vực trong bảng độ sáng
const int so_khu = 15;
int bang_muc_sang[so_khu][2] = {
    {0, 0}, {20, 50}, {50, 100}, {100, 200}, {100, 150}, {150, 250},
    {200, 400}, {250, 350}, {300, 500}, {500, 700}, {750, 850},
    {1500, 2000}, {2000, 5000}, {5000, 10000}, {10000, 20000}
};
// Hàm ghi lỗi ra file log và thời gian xuất hiện lỗi
void ghi_loi(const char* noi_dung_loi) {
    FILE* tep_log = fopen(file_ghi_loi, "a");
    if (tep_log) {
        time_t bay_gio = time(NULL);
        struct tm* tg = localtime(&bay_gio);
        fprintf(tep_log, "[%04d-%02d-%02d %02d:%02d:%02d] %s\n",
                tg->tm_year + 1900, tg->tm_mon + 1, tg->tm_mday,
                tg->tm_hour, tg->tm_min, tg->tm_sec, noi_dung_loi);
        fclose(tep_log);
    }
    fprintf(stderr, "%s\n", noi_dung_loi);
}
// Hàm kiểm tra định dạng thời gian
int la_dinh_dang_thoi_gian(const char* chuoi_thoi_gian) {
    return (strlen(chuoi_thoi_gian) == 19 &&
            chuoi_thoi_gian[4] == ':' && chuoi_thoi_gian[7] == ':' &&
            chuoi_thoi_gian[10] == ' ' && chuoi_thoi_gian[13] == ':' &&
            chuoi_thoi_gian[16] == ':');
}
// Hàm lấy mốc giờ tính trung bình (ví dụ: 07:23 sẽ lấy thành 08:00:00)
void lay_moc_gio(const char* thoi_gian_day_du, char* thoi_gian_moc) {
    struct tm tg = {};
    sscanf(thoi_gian_day_du, "%d:%d:%d %d:%d:%d",
           &tg.tm_year, &tg.tm_mon, &tg.tm_mday,
           &tg.tm_hour, &tg.tm_min, &tg.tm_sec);
    tg.tm_year -= 1900;
    tg.tm_mon -= 1;
    tg.tm_hour += 1;
    tg.tm_min = 0;
    tg.tm_sec = 0;
    mktime(&tg); // chuẩn hóa lại struct tm nếu quá 23h
    // In ra theo định dạng YYYY:MM:DD hh:00:00
    snprintf(thoi_gian_moc, 20, "%04d:%02d:%02d %02d:00:00",
             tg.tm_year + 1900, tg.tm_mon + 1, tg.tm_mday, tg.tm_hour);
}
// Hàm chuyển chuỗi thành số nguyên
int chuyen_chuoi_thanh_so(const char* chuoi) {
    int so = 0;
    for (int i = 0; chuoi[i] >= '0' && chuoi[i] <= '9'; i++) {
        so = so * 10 + (chuoi[i] - '0');
    }
    return so;
}
// Hàm chuyển chuỗi thành số thực 
float chuyen_chuoi_thanh_float(const char* chuoi) {
    float ket_qua = 0.0f;
    int phan_nguyen = 0, i = 0;
    while (chuoi[i] >= '0' && chuoi[i] <= '9') {
        phan_nguyen = phan_nguyen * 10 + (chuoi[i] - '0');
        i++;
    }
    if (chuoi[i] == '.') i++;
    float he_so = 0.1f;
    while (chuoi[i] >= '0' && chuoi[i] <= '9') {
        ket_qua += (chuoi[i] - '0') * he_so;
        he_so *= 0.1f;
        i++;
    }
    return phan_nguyen + ket_qua;
}
// Hàm tách 1 dòng dữ liệu từ file valid thành 3 loại dữ liệu: id, thời gian, giá trị
void tach_dong(const char* dong, char* cot1, char* cot2, char* cot3) {
    int cot = 0, j = 0;
    for (int i = 0; dong[i] != '\0'; ++i) {
        if (dong[i] == ',' || dong[i] == '\n') {
            if (cot == 0) cot1[j] = '\0';
            else if (cot == 1) cot2[j] = '\0';
            else if (cot == 2) cot3[j] = '\0';
            cot++; j = 0;
        } else {
            if (cot == 0) cot1[j++] = dong[i];
            else if (cot == 1) cot2[j++] = dong[i];
            else if (cot == 2) cot3[j++] = dong[i];
        }
    }
    if (cot == 2) cot3[j] = '\0';
}
// Hàm tách 1 dòng dữ liệu từ file location thành 2 loại dữ liệu: id, vị trí
void tach_dong2(const char* dong, char* cot1, char* cot2) {
    int cot = 0, j = 0;
    for (int i = 0; dong[i] != '\0'; ++i) {
        if (dong[i] == ',' || dong[i] == '\n') {
            if (cot == 0) cot1[j] = '\0';
            else if (cot == 1) cot2[j] = '\0';
            cot++; j = 0;
        } else {
            if (cot == 0) cot1[j++] = dong[i];
            else if (cot == 1) cot2[j++] = dong[i];
        }
    }
    if (cot == 1) cot2[j] = '\0';
}
// Task 2.1 để xử lý và lọc dữ liệu từ file CSV, tách các outlier và valid đồng thời chúng vào các file csv với tên gọi tương ứng
// Hàm này cũng sẽ ghi lại các lỗi có thể phát sinh trong quá trình xử lý dữ liệu
int task2_1() {
    FILE* tep_vao = fopen(ten_file_dau_vao, "r");
    if (!tep_vao) {
        ghi_loi("Error 01: input file not found or not accessible");
        return 0;
    }
    FILE* tep_hop_le = fopen(file_gia_tri_hop_le, "w");
    if (!tep_hop_le) {
        ghi_loi("Error 07: cannot override output file");
        fclose(tep_vao);
        return 0;
    }
    char du_lieu_ngoai_le[10000][100];
    int so_ngoai_le = 0;
    char dong[256];
    fgets(dong, sizeof(dong), tep_vao);
    fprintf(tep_hop_le, "id,time,value\n");

    int so_dong = 0;
    while (fgets(dong, sizeof(dong), tep_vao)) {
        so_dong++;
        if (so_dong > max_so_dong) {
            ghi_loi("Error 08: input file is too large");
            break;
        }
        char id[20], thoi_gian[30], gia_tri[20];
        tach_dong(dong, id, thoi_gian, gia_tri);

        if (!*id || !*thoi_gian || !*gia_tri || !la_dinh_dang_thoi_gian(thoi_gian)) {
            ghi_loi("Error 04: invalid data at line");
            continue;
        }
        int ma_cb = chuyen_chuoi_thanh_so(id);
        float lux = chuyen_chuoi_thanh_float(gia_tri);
        if (lux < 1.0 || lux > 35000.0) {
            sprintf(du_lieu_ngoai_le[so_ngoai_le], "%d,%s,%.2f", ma_cb, thoi_gian, lux);
            so_ngoai_le++;
        } else {
            fprintf(tep_hop_le, "%d,%s,%.2f\n", ma_cb, thoi_gian, lux);
            du_lieu_hop_le[so_dong_hop_le].id = ma_cb;
            strcpy(du_lieu_hop_le[so_dong_hop_le].thoi_gian, thoi_gian);
            du_lieu_hop_le[so_dong_hop_le].lux = lux;
            so_dong_hop_le++;
        }
    }
    fclose(tep_vao);
    fclose(tep_hop_le);
    FILE* tep_ngoai_le = fopen(file_gia_tri_ngoai_le, "w");
    if (!tep_ngoai_le) {
        ghi_loi("Error 07: cannot override output file");
        return 0;
    }
    fprintf(tep_ngoai_le, "number of outliers: %d\n", so_ngoai_le);
    fprintf(tep_ngoai_le, "id,time,value\n");
    for (int i = 0; i < so_ngoai_le; i++) {
        fprintf(tep_ngoai_le, "%s\n", du_lieu_ngoai_le[i]);
    }
    fclose(tep_ngoai_le);
    return 1;
}
// Task 2.2 để phân loại điều kiện sáng tuỳ theo vị trí của file location.csv, đồng thời ghi lại các lỗi có thể phát sinh
// Hàm đánh giá điều kiện ánh sáng theo từng giờ trung bình
// Mỗi cảm biến được tính trung bình lux mỗi giờ, sau đó phân loại điều kiện ánh sáng dựa trên khu vực
void task2_2() {
    // Mở file ánh xạ cảm biến sang khu vực
    FILE* tep_vitri = fopen(ten_file_vi_tri, "r");
    if (!tep_vitri) {
        ghi_loi("Error 01: input file not found or not accessible");
        return;
    }
    // Khởi tạo mảng ánh xạ id cảm biến → mã khu vực
    int cam_bien_sang_khu[101] = {0};
    char dong[64];
    fgets(dong, sizeof(dong), tep_vitri); // Bỏ dòng tiêu đề

    while (fgets(dong, sizeof(dong), tep_vitri)) {
        char id[20], vitri[20];
        tach_dong2(dong, id, vitri);
        // Nếu thiếu dữ liệu thì ghi lỗi Error 04 và tiếp tục duyệt tiếp
        if (!*id || !*vitri) {
            ghi_loi("Error 04: invalid data in location file");
            continue;
        }
        int ma_cb = chuyen_chuoi_thanh_so(id);
        int ma_khu = chuyen_chuoi_thanh_so(vitri);
        cam_bien_sang_khu[ma_cb] = ma_khu;
    }
    fclose(tep_vitri);
    // Cấu trúc để lưu tổng lux và số lần theo id + mốc giờ
    struct TrungBinhTheoGio {
        int id;
        char gio[20];
        float tong_lux;
        int dem;
    } trungbinh[10000];
    int so_moc = 0;
    // Gom các dữ liệu theo id và mốc giờ
    for (int i = 0; i < so_dong_hop_le; i++) {
        char gio_moc[20];
        lay_moc_gio(du_lieu_hop_le[i].thoi_gian, gio_moc);
        int da_co = 0;
        for (int j = 0; j < so_moc; j++) {
            if (trungbinh[j].id == du_lieu_hop_le[i].id &&
                strcmp(trungbinh[j].gio, gio_moc) == 0) {
                trungbinh[j].tong_lux += du_lieu_hop_le[i].lux;
                trungbinh[j].dem++;
                da_co = 1;
                break;
            }
        }
        if (!da_co) {
            trungbinh[so_moc].id = du_lieu_hop_le[i].id;
            strcpy(trungbinh[so_moc].gio, gio_moc);
            trungbinh[so_moc].tong_lux = du_lieu_hop_le[i].lux;
            trungbinh[so_moc].dem = 1;
            so_moc++;
        }
    }
    // Sắp xếp theo thời gian trước, id sau
    for (int i = 0; i < so_moc - 1; i++) {
        for (int j = i + 1; j < so_moc; j++) {
            if (strcmp(trungbinh[i].gio, trungbinh[j].gio) > 0 ||
                (strcmp(trungbinh[i].gio, trungbinh[j].gio) == 0 && trungbinh[i].id > trungbinh[j].id)) {
                struct TrungBinhTheoGio tam = trungbinh[i];
                trungbinh[i] = trungbinh[j];
                trungbinh[j] = tam;
            }
        }
    }
    // Ghi ra file kết quả
    FILE* tep_kq = fopen(file_dieu_kien, "w");
    if (!tep_kq) {
        ghi_loi("Error 07: cannot override output file");
        return;
    }
    fprintf(tep_kq, "id,time,location,value,condition\n");
    for (int i = 0; i < so_moc; i++) {
        int id = trungbinh[i].id;
        float trung_binh = trungbinh[i].tong_lux / trungbinh[i].dem;
        int khu = cam_bien_sang_khu[id];

        const char* nhan = "NA";
        if (khu > 0 && khu < so_khu) {
            int min = bang_muc_sang[khu][0];
            int max = bang_muc_sang[khu][1];
            if (trung_binh < min) nhan = "dark";
            else if (trung_binh > max) nhan = "bright";
            else nhan = "good";
        }

        fprintf(tep_kq, "%d,%s,%d,%.2f,%s\n",
                id, trungbinh[i].gio, khu, trung_binh, nhan);
    }
    fclose(tep_kq);
}
// Task 2.3: Tổng hợp các giá trị max, min, mean cho từng id và ghi lại lỗi nếu có
void task2_3() {
    float tong[101] = {0};
    int dem[101] = {0};
    float lon_nhat[101] = {0}, nho_nhat[101] = {999999};
    char tg_max[101][20], tg_min[101][20];
    time_t thoi_gian_dau = 0, thoi_gian_cuoi = 0;
    for (int i = 0; i < so_dong_hop_le; i++) {
        int id = du_lieu_hop_le[i].id;
        tong[id] += du_lieu_hop_le[i].lux;
        dem[id]++;
        if (du_lieu_hop_le[i].lux > lon_nhat[id] || dem[id] == 1)
            lon_nhat[id] = du_lieu_hop_le[i].lux, strcpy(tg_max[id], du_lieu_hop_le[i].thoi_gian);
        if (du_lieu_hop_le[i].lux < nho_nhat[id] || dem[id] == 1)
            nho_nhat[id] = du_lieu_hop_le[i].lux, strcpy(tg_min[id], du_lieu_hop_le[i].thoi_gian);
        // Cập nhật thời gian bắt đầu và kết thúc mô phỏng
        struct tm tg_tm = {};
        sscanf(du_lieu_hop_le[i].thoi_gian, "%d:%d:%d %d:%d:%d",
            &tg_tm.tm_year, &tg_tm.tm_mon, &tg_tm.tm_mday,
            &tg_tm.tm_hour, &tg_tm.tm_min, &tg_tm.tm_sec);
        tg_tm.tm_year -= 1900;
        tg_tm.tm_mon -= 1;
        time_t tg_hien_tai = mktime(&tg_tm);
        if (thoi_gian_dau == 0 || tg_hien_tai < thoi_gian_dau)
            thoi_gian_dau = tg_hien_tai;
        if (tg_hien_tai > thoi_gian_cuoi)
            thoi_gian_cuoi = tg_hien_tai;
    }
    // Tính thời gian mô phỏng thực tế
    int tong_giay = (int)(thoi_gian_cuoi - thoi_gian_dau);
    int gio = tong_giay / 3600;
    int phut = (tong_giay % 3600) / 60;
    int giay = tong_giay % 60;

    char chuoi_thoi_gian_tb[20];
    sprintf(chuoi_thoi_gian_tb, "%02d:%02d:%02d", gio, phut, giay);
    FILE* tep_tongket = fopen(file_tong_ket, "w");
    if (!tep_tongket) {
        ghi_loi("Error 07: cannot override output file");
        return;
    }
    fprintf(tep_tongket, "id,parameter,time,value\n");
    for (int i = 1; i < 101; i++) {
        if (dem[i]) {
            fprintf(tep_tongket, "%d,max,%s,%.2f\n", i, tg_max[i], lon_nhat[i]);
            fprintf(tep_tongket, "%d,min,%s,%.2f\n", i, tg_min[i], nho_nhat[i]);
            fprintf(tep_tongket, "%d,mean,%s,%.2f\n", i, chuoi_thoi_gian_tb, tong[i]/dem[i]);
        }
    }
    fclose(tep_tongket);
}
// Gọi lại các hàm task2_1, task2_2, task2_3
int main() {
    FILE* log_reset = fopen(file_ghi_loi, "w");
    if (log_reset) fclose(log_reset);
    if (!task2_1()) return 0;
    printf("Da hoan thanh task 2.1\n");
    task2_2();
    printf("Da hoan thanh task 2.2\n");
    task2_3();
    printf("Da hoan thanh task 2.3. Vui long kiem tra file ket qua.\n");
    return 0;
}
