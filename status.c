int last_return_status;

int get_return_status() {
    return last_return_status;
}

void set_return_status(int status) {
    last_return_status = status;
}
