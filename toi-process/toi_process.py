import os
import argparse

def copy_file(src_file_name, dst_file_name):
    # Copy src_filep to toi_filep
    buffer = ""

    try:
        with open(src_file_name, "r") as src_filep:
            with open(dst_file_name, "w") as dst_filep:
                for line in src_filep:
                    dst_filep.write(line)
    except IOError as e:
        print(f"Failed to open file: {e}")

def make_one_epc_unique(file_name, line_number_tag):
    buffer = ""
    c_epc = ""
    epc_length = 24  # preset 96-bit (24-nibble) by default
    line_cnt = 0
    multi_read_times = 0  # The times of one tag was read multi-times

    if line_number_tag == 0:
        print("Making unique epc cannot from line 0!")
        return 0xFFFF

    try:
        with open(file_name, "r") as filep:
            lines = filep.readlines()

        with open("file_tmp.txt", "w") as file_tmp:
            for line in lines:
                if line == "\n":
                    break
                line_cnt += 1
                if line_cnt < line_number_tag:
                    file_tmp.write(line)
                    continue
                else:
                    plast = line.find(",T5,")
                    c_epc_length = line[plast+4:plast+6]
                    epc_length = int(c_epc_length) * 2
                    if epc_length > 64:
                        print("The length of EPC is more than 256 bits, "
                              "exit counting the unique tags!")
                        line_cnt = 0xFFFF
                        break
                    if line_cnt == line_number_tag:
                        c_epc = line[plast+7:plast+7+epc_length]
                        file_tmp.write(line)
                    else:
                        if c_epc not in line:
                            file_tmp.write(line)
                        else:
                            line_cnt -= 1
                            multi_read_times += 1

        os.remove(file_name)
        os.rename("file_tmp.txt", file_name)
        print("multi_read_times: %d\n", multi_read_times)

        # Add the times of multi_read at the unique tag line
        with open(file_name, "r") as filep:
            lines = filep.readlines()

        with open("file_tmp.txt", "w") as file_tmp:
            current_line = 0
            for line in lines:
                if line == "\n":
                    break
                current_line += 1
                if current_line == line_number_tag:
                    line = line.rstrip("\n")
                    file_tmp.write(f"{line},{multi_read_times + 1}\n")
                else:
                    file_tmp.write(line)

        os.remove(file_name)
        os.rename("file_tmp.txt", file_name)

    except IOError as e:
        print(f"Failed to open file: {e}")

    return line_cnt

def unique_tag_count(src_file_name, toi_file_name):
    copy_file(src_file_name, toi_file_name)

    header_line_number = 6
    total_line_number = 0
    number_toi = 0 + header_line_number

    while number_toi < 10000:
        number_toi += 1
        total_line_number = make_one_epc_unique(toi_file_name, number_toi)
        print(f"Processed line {number_toi}, total line number: {total_line_number}")
        if total_line_number == 0:
            number_toi -= 1
            break
        elif total_line_number == number_toi:
            break
        elif total_line_number == 0xFFFF:
            break

    if total_line_number != 0xFFFF:
        number_toi -= header_line_number
        print(f"{number_toi} tags singulated.")

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Process TOI data")
    parser.add_argument("-s", "--src", help="Source file name", required=True)
    parser.add_argument("-d", "--dst", help="Destination file name", required=True)

    args = parser.parse_args()
    unique_tag_count(args.src, args.dst)