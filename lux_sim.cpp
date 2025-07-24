#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <cstring>
#include <cctype>
// Các thông số mặc định nếu đầu vào thiếu tất cả các tham số yêu cầu
const int sensor_default = 1;
const int sampling_default = 60;
const int interval_default = 24;
const char* file_log = "task1.log";
const char* file_output = "lux_sensor.csv";
// Kiểm tra xem ký tự có phải là chữ số hợp lệ 
bool laKyTuSo(char c) {
    return (c >= '0' && c <= '9');
}
// Đưa chuỗi về chữ thường để xử lý các flag không phân biệt chữ hoa/thường
void chuyenChuoiVeThuong(char* str) {
    for (int i = 0; str[i]; ++i) {
        if (str[i] >= 'A' && str[i] <= 'Z') {
            str[i] = str[i] - 'A' + 'a';
        }
    }
}
// Định dạng thời gian theo chuẩn đề bài: yyyy:mm:dd hh:mm:ss
void dinhDangThoiGianChuoi(time_t thoiGian, char* ketQua) {
    tm* info = localtime(&thoiGian);
    int nam = 1900 + info->tm_year;
    int thang = 1 + info->tm_mon;
    int ngay = info->tm_mday;
    int gio = info->tm_hour;
    int phut = info->tm_min;
    int giay = info->tm_sec;
    sprintf(ketQua, "%04d:%02d:%02d %02d:%02d:%02d", nam, thang, ngay, gio, phut, giay);
}
// Ghi lỗi có timestamp vào file log
void ghiLoiVaoFileLog(const char* noiDung) {
    FILE* log = fopen(file_log, "a");
    if (log != NULL) {
        time_t now = time(0);
        tm* ltm = localtime(&now);
        char buf[20];
        sprintf(buf, "%04d-%02d-%02d %02d:%02d:%02d",
                1900 + ltm->tm_year, 1 + ltm->tm_mon, ltm->tm_mday,
                ltm->tm_hour, ltm->tm_min, ltm->tm_sec);
        fprintf(log, "[%s] %s\n", buf, noiDung);
        fclose(log);
    }
    fprintf(stderr, "%s\n", noiDung); // luôn in ra stderr để debug
}
// Hàm kiểm tra chuỗi là số nguyên và chuyển đổi thành int, giá trị của biến ketQua sẽ được cập nhật mỗi lần kiểm tra
bool chuyenChuoiSangSo(const char* dauVao, int& ketQua) {
    if (!dauVao || strlen(dauVao) == 0) return false;
    int i = 0;
    bool laSoAm = false;
    if (dauVao[0] == '-') {
        laSoAm = true;
        i = 1;
        if (strlen(dauVao) == 1) return false;
    }
    ketQua = 0;
    for (; dauVao[i]; ++i) {
        if (!laKyTuSo(dauVao[i])) return false;
        ketQua = ketQua * 10 + (dauVao[i] - '0');
    }
    if (laSoAm) ketQua = -ketQua;
    return true;
}
// Phân tích và xác thực tham số dòng lệnh (nếu sai ghi lỗi)
bool xuLyThamSo(int soThamSo, char* danhSachThamSo[], int& soLuongCamBien, int& buocLayMau, int& thoiLuong) {
    soLuongCamBien = sensor_default;
    buocLayMau = sampling_default;
    thoiLuong = interval_default;
    bool coN = false, coS = false, coI = false;
    int nVal = 0, sVal = 0, iVal = 0;
    for (int i = 1; i < soThamSo; ++i) {
        chuyenChuoiVeThuong(danhSachThamSo[i]);
//Điều kiện kiểm tra xem 1 trong 3 tham số có bị thiếu hay không
        if (strcmp(danhSachThamSo[i], "-n") == 0 ||
            strcmp(danhSachThamSo[i], "-s") == 0 ||
            strcmp(danhSachThamSo[i], "-i") == 0) {

            if (i + 1 >= soThamSo) {
                // Lỗi nếu thiếu giá trị sau 1 tham số
                ghiLoiVaoFileLog("Error 01: invalid command");
                return false;
            }
            int giaTri;
            if (!chuyenChuoiSangSo(danhSachThamSo[i + 1], giaTri)) {
                ghiLoiVaoFileLog("Error 01: invalid command");
                return false;
            }
            if (strcmp(danhSachThamSo[i], "-n") == 0) {
                coN = true; nVal = giaTri;
            } else if (strcmp(danhSachThamSo[i], "-s") == 0) {
                coS = true; sVal = giaTri;
            } else if (strcmp(danhSachThamSo[i], "-i") == 0) {
                coI = true; iVal = giaTri;
            }
            i++; // bỏ qua tham số đã xử lý
        } else {
            ghiLoiVaoFileLog("Error 01: invalid command");
            return false;
        }
    }
    // Nếu thiếu 1 trong 3 tham số thì coi như lỗi command tức là lỗi 1 và sẽ ghi vào log
    if (!(coN && coS && coI)) {
        ghiLoiVaoFileLog("Error 01: invalid command");
        return false;
    }
    // Nếu đã có đủ cả 3 tham số nhưng có 1 giá trị không dương (<=0) thì coi như lỗi argument
    if (nVal <= 0 || sVal <= 0 || iVal <= 0) {
        ghiLoiVaoFileLog("Error 02: invalid argument");
        return false;
    }
    soLuongCamBien = nVal;
    buocLayMau = sVal;
    thoiLuong = iVal;
    return true;
}
// Sinh giá trị lux ngẫu nhiên 2 chữ số thập phân
float sinhLuxNgauNhien2ChuSo() {
    int phanNguyen = rand() % 1000000 + 1;
    int phanThapPhan = rand() % 100;
    return phanNguyen / 100.0f + phanThapPhan / 10000.0f;
}
// Hàm mô phỏng và ghi dữ liệu vào file CSV
void sinhVaXuatGiaTriMoPhong(FILE* tepDauRa, int camBien, int buocMau, int soGio) {
    time_t hienTai = time(NULL);
    time_t batDau = hienTai - soGio * 3600;
    int tongSoMau = (soGio * 3600) / buocMau;
    for (int i = 0; i <= tongSoMau; ++i) {
        time_t mocThoiGian = batDau + i * buocMau;
        char chuoiThoiGian[20];
        dinhDangThoiGianChuoi(mocThoiGian, chuoiThoiGian);
        for (int maCamBien = 1; maCamBien <= camBien; ++maCamBien) {
            float giaTriLux = sinhLuxNgauNhien2ChuSo();
            fprintf(tepDauRa, "%d,%s,%.2f\n", maCamBien, chuoiThoiGian, giaTriLux);
        }
    }
}
// Chương trình chính
int main(int soThamSo, char* danhSachThamSo[]) {
    // Mỗi lần chạy xóa log cũ
    FILE* logReset = fopen(file_log, "w");
    if (logReset) fclose(logReset);
    srand((unsigned int)time(NULL)); // khởi tạo seed ngẫu nhiên
    int camBien = 0, buocMau = 0, soGio = 0;
    if (!xuLyThamSo(soThamSo, danhSachThamSo, camBien, buocMau, soGio)) {
        printf("Phat hien loi, hay kiem tra file log de biet them chi tiet.\n");
        return 0;
    }

    FILE* tepDauRa = fopen(file_output, "w");
    if (!tepDauRa) {
        ghiLoiVaoFileLog("Error 03: file access denied");
        printf("Phat hien loi, hay kiem tra file log de biet them chi tiet.\n");
        return 0;
    }
    fprintf(tepDauRa, "id,time,value\n");
    sinhVaXuatGiaTriMoPhong(tepDauRa, camBien, buocMau, soGio);
    fclose(tepDauRa);
    printf("Mo phong da hoan tat, hay kiem tra file sau: %s\n", file_output);
    return 0;
}
