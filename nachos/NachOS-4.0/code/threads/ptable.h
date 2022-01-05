//ptable.h  
#ifndef PTABLE_H 
#define PTABLE_H 
#include "bitmap.h" 
#include "pcb.h"
#include "semaphore.h" 
#define MAXPROCESS 10
#define MAX_PROCESS_NAME_LENGTH 32

class PTable { 
private: 
    Bitmap *bm; 
    PCB *pcb[MAXPROCESS]; 
    int psize; 
    Semaphore *bmsem; // dùng để ngăn chặn trường hợp nạp 2 tiến trình cùng lúc.
public: 
    PTable(int size); 
    // Khởi tạo size đối tượng pcb để lưu size process. Gán giá trị ban đầu là null. Nhớ khởi tạo *bm và *bmsem để sử dụng.

    ~PTable();
    //Hủy các đối tượng đã tạo.

    int ExecUpdate(char* fileName); //return PID
    //Thực thi cho system call SC_EXEC, kiểm tra chương trình được gọi có tồn tại trong máy không. Kiểm tra thử xem chương trình gọi lại chính nó không? Chúng ta không cho phép điều này. Kiểm tra còn slot trống để lưu tiến trình mới không (max là 10 process). Nếu thỏa các điều kiện trên thì ta lấy index của slot trống là processID cảu tiền trình mới tạo này, giả sử là ID. Và gọi phương thức EXEC của lớp PCB với đối tượng tương ứng quản lý process này, nghĩa là gọi pcb[ID]->Exec(…). Xem chi tiết mo tả trong lớp PCB ở bên dưới.

    int ExitUpdate(int ec);
    //Được sử dụng bởi SC_Exit. Trong system call này chúng ta kiểm tra thử PID có tồn tại không, sau đó chúng ta mới xử lý kết thúc tiến trình, phải gọi JoinRelease để giải phòng sự chờ đợi của tiến trình cha và ExitWait để xin phép tiến trình cha cho kết thúc. Chú ý là sau khi gọi thủ tục này, chúng ta phải giải phóng tiểu trình hiện tại bằng các dòng lệnh như sau nếu là main process thì chúng ta gọi hàm Halt luôn

    int JoinUpdate(int id);
    //Được sử dụng cho system call Join(), trong thủ tục này thì tiến trình cha goi pcb[i]->JoinWait() để chờ cho tới khi tiến trình con kết thúc (trước khi tiến trình con kết thúc thì tiến trình con gọi JoinRelease()). Sau đó tiến trình cha gọi ExitRelease() để cho phép tiến trình con được kết thúc.
    // Chú ý là ta không được cho phép tiến trình join vào chính nó, hoặc join vào tiến trình khác không phải là cha của nó.

    int GetFreeSlot();
    //Tìm free slot để lưu thông tin cho tiến trình mới.

    bool IsExist(int pid);
    //Kiểm tra có tồn tại processId này không

    void Remove(int pid);
    //Xóa một processID ra khỏng mãng quản lý nó, khi mà tiến trình này kết thúc.
    
    char* GetFileName(int id); 
    // Trả về tên của tiến trình
};
#endif