import pygame
from pygame.locals import *
import sys
import os

class App:
    def detect_device_type(self):
        try:
            with open('/tmp/deviceModel', 'r') as f:
                device_id = f.read().strip()
                if device_id == '354':
                    return 'Miyoo Mini Plus'
                if device_id == '285':
                    return 'Miyoo Mini Flip'
                elif device_id == '283':
                    return 'Miyoo Mini'
        except:
            pass
        return 'Unknown'

    def show_device_selection(self):
        pygame.draw.rect(self.screen, self.menu_item_color, (0, 0, self.app_width, self.app_height))
        self.draw_text(self.font, self.white, self.app_width // 2, 8, 'center', 'Select Your Device')
        devices = ['Miyoo Mini', 'Miyoo Flip', 'Miyoo Mini Plus']
        selected_index = 0

        line_spacing = self.font_size + 14
        background_height = self.font_size + 10

        while True:
            pygame.draw.rect(self.screen, self.menu_item_color, (0, 0, self.app_width, self.app_height))
            self.draw_text(self.font, self.white, self.app_width // 2, 8, 'center', 'Select Your Device')

            for i, device in enumerate(devices):
                y = 40 + i * line_spacing
                if i == selected_index:
                    selection_rect = pygame.Rect(8, y - 2, self.app_width - 16, background_height)
                    pygame.draw.rect(self.screen, self.white, selection_rect)
                    self.draw_text(self.font, (0, 0, 0), 12, y + 2, 'left', device)
                else:
                    self.draw_text(self.font, self.white, 12, y + 2, 'left', device)
            self.draw_text(self.font, self.gray, self.app_width // 2, self.app_height - 40, 'center', 'A: Select   Up/Down: Navigate')
            pygame.display.flip()
            for event in pygame.event.get():
                if event.type == KEYDOWN:
                    if event.key == K_UP:
                        selected_index = (selected_index - 1) % len(devices)
                    elif event.key == K_DOWN:
                        selected_index = (selected_index + 1) % len(devices)
                    elif event.key in [K_RETURN, K_LCTRL, K_SPACE]:
                        return devices[selected_index]
                elif event.type == QUIT:
                    pygame.quit()
                    sys.exit()
            pygame.time.wait(10)
    def draw_text(self, font, color, x, y, align='left', text=''):
        text_obj = font.render(text, True, color)
        text_rect = text_obj.get_rect()
        if align == 'center':
            text_rect.midtop = (x, y)
        elif align == 'right':
            text_rect.topright = (x, y)
        else:
            text_rect.topleft = (x, y)
        self.screen.blit(text_obj, text_rect)
    def draw_scrollbox(self, x, y, width, percent):
        height = self.app_height - y - 30
        scrollbox_width = 4
        scrollbox_height = height // (self.memo_lines_max / self.memo_lines_ouput_max)
        scrollbox_y = y + (height - scrollbox_height) * (percent / 100.0)
        pygame.draw.rect(self.screen, self.scrollbox_background_color, (x, y, scrollbox_width, height), 0)
        if percent != -1:
            pygame.draw.rect(self.screen, self.white, (x, scrollbox_y, scrollbox_width, scrollbox_height), 0)
    def read_cpu_clock(self):
        clock_file = os.path.join(self.config_dir, 'cpuclock.txt')
        if os.path.exists(clock_file):
            with open(clock_file, 'r') as f:
                return f.read().strip()
        return '1200'
    def __init__(self):
        self.memo = ""
        self.memo_line_offset = 0
        self.memo_lines_max = 0
        self.memo_lines_ouput_max = 10
        if len(sys.argv) > 1:
            self.config_dir = ' '.join(sys.argv[1:])
        else:
            print("Error: No core path")
            sys.exit(1)
        pygame.init()
        pygame.font.init()
        self.app_width = 640
        self.app_height = 480
        self.screen = pygame.display.set_mode((self.app_width, self.app_height))
        pygame.display.set_caption('App')
        self.background = pygame.Surface(self.screen.get_size()).convert()
        self.background.fill((7, 18, 22))
        self.black = pygame.Color(0, 0, 0)
        self.white = pygame.Color(255, 255, 255)
        self.gray = pygame.Color(128, 128, 128)
        self.green = pygame.Color(0, 255, 0)
        self.red = pygame.Color(255, 0, 0)
        self.menu_item_color = pygame.Color(58, 69, 73)
        self.memo_background_color = pygame.Color(28, 35, 40)
        self.scrollbox_background_color = pygame.Color(60, 70, 72)
        self.font_size = 25
        self.font = pygame.font.Font('/customer/app/wqy-microhei.ttc', self.font_size)
        self.device_type = self.detect_device_type()
        if self.device_type == 'Unknown':
            self.device_type = self.show_device_selection()
        self.fps_controller = pygame.time.Clock()
        self.layout_index = 0
        self.goodbye_start_time = 0
        self.status_message = ''
        self.status_color = self.white
        self.status_start_time = 0
        if self.device_type == 'Miyoo Mini Plus':
            self.main_list = [str(x) for x in range(600, 1900, 100)]
        elif self.device_type == 'Miyoo Mini Flip':
            self.main_list = [str(x) for x in range(600, 1900, 100)]
        else:
            self.main_list = [str(x) for x in range(600, 1700, 100)]
        self.main_list_output_max = 10
        self.list_selected_index = 0
        self.list_selected_offset = 0
        self.list_selected_item = ''
        self.current_clock = self.read_cpu_clock()
        self.main_loop()
    def main_loop(self):
        while True:
            self.handle_events()
            self.update_screen()
            self.fps_controller.tick(30)
    def write_cpu_clock(self, clock_speed):
        try:
            clock_file = os.path.join(self.config_dir, 'cpuclock.txt')
            with open(clock_file, 'w') as f:
                f.write(clock_speed)
            return True
        except Exception as e:
            print("Write error:", e)
            return False
    def handle_events(self):
        for event in pygame.event.get():
            if event.type == QUIT:
                pygame.quit()
                sys.exit()
            elif event.type == KEYDOWN:
                if event.key == K_LCTRL:
                    self.layout_index = 2
                    self.goodbye_start_time = pygame.time.get_ticks()
                elif self.layout_index == 0:
                    if event.key == K_UP:
                        if self.list_selected_index > 0:
                            self.list_selected_index -= 1
                        else:
                           self.list_selected_index = len(self.main_list) - 1
                    elif event.key == K_DOWN:
                        if self.list_selected_index < len(self.main_list) - 1:
                            self.list_selected_index += 1
                        else:
                            self.list_selected_index = 0
                    elif event.key == K_RETURN or event.key == K_SPACE:
                        self.list_selected_item = self.main_list[self.list_selected_index]
                        if self.write_cpu_clock(self.list_selected_item):
                            self.current_clock = self.list_selected_item
                            self.status_message = 'successfully!'
                            self.status_color = self.green
                        else:
                            self.status_message = 'Failed (IO Error)'
                            self.status_color = self.red
                        self.status_start_time = pygame.time.get_ticks()
                    elif event.key == K_LSHIFT:  # X button
                        if self.delete_cpu_clock_file():
                           self.status_message = 'Use default!'
                           self.status_color = self.green
                           self.current_clock = self.read_cpu_clock()
                        else:
                           self.status_message = 'default'
                           self.status_color = self.red
                        self.status_start_time = pygame.time.get_ticks()
                    if self.list_selected_index > self.main_list_output_max - 1:
                        self.list_selected_offset = self.list_selected_index // self.main_list_output_max * self.main_list_output_max
                    else:
                        self.list_selected_offset = 0
                elif self.layout_index == 1:
                    if event.key == K_ESCAPE or event.key == K_BACKSPACE:
                        self.layout_index = 0
                    elif event.key == K_UP:
                        if self.memo_line_offset > 0:
                            self.memo_line_offset -= 1
                    elif event.key == K_DOWN:
                        if self.memo_line_offset < self.memo_lines_max - self.memo_lines_ouput_max:
                            self.memo_line_offset += 1
    def delete_cpu_clock_file(self):
        try:
            if hasattr(self, 'config_dir') and self.config_dir:
                clock_file = os.path.join(self.config_dir, 'cpuclock.txt')
                if os.path.exists(clock_file):
                    os.remove(clock_file)
                    return True
            return False
        except Exception as e:
            return False
    def update_screen(self):
        self.screen.fill((0, 0, 0))
        if self.layout_index == 0:
            self.draw_layout_list()
        elif self.layout_index == 1:
            self.draw_layout_memo()
        elif self.layout_index == 2:
            self.draw_goodbye()
            if pygame.time.get_ticks() - self.goodbye_start_time >= 2000:
                pygame.quit()
                sys.exit()
        pygame.display.flip()
    def draw_layout_list(self):
        core_name = os.path.basename(self.config_dir) if hasattr(self, 'config_dir') else ""
        title = 'CPUSpeeds'
        if core_name:
            title += u' : ' + core_name + u''
        self.draw_text(self.font, self.white, 8, 8, 'left', title)
        self.draw_text(self.font, self.gray, self.app_width - 8, 8, 'right', self.device_type)
        if self.status_message and pygame.time.get_ticks() - self.status_start_time < 2000:
            self.draw_text(self.font, self.status_color, self.app_width - 8, self.app_height - 40, 'right', self.status_message)
        line_spacing = self.font_size + 14
        background_height = self.font_size + 10
        for i, line in enumerate(self.main_list):
            if i < self.list_selected_offset:
                continue
            if i - self.list_selected_offset >= self.main_list_output_max:
                continue
            if i == self.list_selected_index:
                item_background_color = self.white
                item_font_color = self.black
            else:
                item_background_color = self.menu_item_color
                item_font_color = self.white
            x = 8
            y = line_spacing * (i - self.list_selected_offset + 1) + 8
            width = self.app_width - 16
            height = background_height
            pygame.draw.rect(self.screen, item_background_color, (x, y, width, height + 3), 0)
            text = self.cut_str(line, 70)
            self.draw_text(self.font, item_font_color, x + 2, y + 4, 'left', text)
            if self.current_clock and line == self.current_clock:
                self.draw_text(self.font, item_font_color, x + width - 4, y + 4, 'right', "Current CPUSpeeds")
        self.draw_text(self.font, self.gray, 8, self.app_height - 40, 'left', 'U/D:Navigate  A:Select  B:Exit  X:Default')
    def draw_layout_memo(self):
        self.draw_text(self.font, self.white, 8, 8, 'left', 'Display: ' + self.list_selected_item)
        self.draw_text(self.font, self.gray, self.app_width - 8, 8, 'right', self.device_type)
        lines = self.add_line_breaks(self.memo, 46).split('\n')
        self.memo_lines_max = len(lines)
        if self.memo_lines_max > self.memo_lines_ouput_max:
            scroll_percent = self.memo_line_offset * 100 // (self.memo_lines_max - self.memo_lines_ouput_max)
        else:
            scroll_percent = -1
        self.draw_scrollbox(self.app_width - 8, 30, 6, scroll_percent)
        self.draw_text(self.font, self.gray, 620, 455, 'right', str(self.memo_line_offset * 100 // (self.memo_lines_max - self.memo_lines_ouput_max)) + '%')
        pygame.draw.rect(self.screen, self.memo_background_color, (9, 30, self.app_width - 24, self.app_height - 60), 0)
        for i, line in enumerate(lines):
            if i < self.memo_line_offset:
                continue
            if i - self.memo_line_offset >= self.memo_lines_ouput_max:
                continue
            self.draw_text(self.font, self.white, 18, self.font_size * (i - self.memo_line_offset) + 40, 'left', line)
    def cut_str(self, string, n):
        if len(string) > n:
            return string[:n-3] + '...'
        else:
            return string
    def add_line_breaks(self, text, n):
        result = ''
        count = 0
        for char in text:
            result += char
            count += 1
            if char == '\n':
                count = 0
                continue
            if count == n:
                count = 0
                text_len = len(result)
                added = False
                for i in range(1, n + 1):
                    if text_len - i < 0:
                        break
                    if result[text_len - i] == ' ':
                        result = result[:text_len - i] + '\n' + result[text_len - i + 1:]
                        added = True
                        break
                if not added:
                    result += '\n'
        return result
    def draw_goodbye(self):
        self.draw_text(self.font, self.white, self.app_width // 2, self.app_height // 2, 'center', 'Exit CPUSpeeds Settings...')
if __name__ == "__main__":
    app = App()
