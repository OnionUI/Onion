
__attribute__((externally_visible))
const char *__asan_default_options() {
  return "log_path=/mnt/SDCARD/.tmp_update/logs/ASAN.log:halt_on_error=0";
}