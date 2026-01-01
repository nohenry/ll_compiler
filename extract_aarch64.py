
from openpyxl import load_workbook
from openpyxl.worksheet.worksheet import Worksheet
from openpyxl.utils import column_index_from_string, rows_from_range
from functools import cmp_to_key
import re


xlsx = load_workbook('/Users/oliverclarke/Downloads/AArch64_ops.xlsx', data_only=True)
sheet = xlsx['Operation_table']
opcode_value_index = column_index_from_string('AU') - 1
opcode_bits_lsb = column_index_from_string('AQ') - 1
opcode_bits_msb = column_index_from_string('L') - 1
opcode_sf = column_index_from_string('L') - 1
opcode_variant = column_index_from_string('I') - 1
opcode_specific = column_index_from_string('H') - 1
opcode_pre = column_index_from_string('F') - 1

opcodes: dict[str, dict[(str, bool), (int, int)]] = {}

last = [None for _ in range(32)]

merged_cells = []
for merged_range in sheet.merged_cells.ranges:
    merged_range = str(merged_range)
    # Convert the range string (e.g., 'A1:C1') into a list of cell coordinates
    merged_cells.append((list(rows_from_range(str(merged_range))), merged_range))

def calculate_span_via_ranges(sheet: Worksheet, target_cell):
    """
    Finds the column span by checking the worksheet's merged cell ranges.
    """
    target_coord = target_cell.coordinate

    for (merged, range) in merged_cells:
        for row in merged:
            if target_coord in row:
                # The target cell is in this merged range
                start_col = sheet[range.split(':')[0]].column
                end_col = sheet[range.split(':')[1]].column
                span = end_col - start_col + 1
                return span
    return 1

for row in sheet.rows:
    variant = []
    
    regname = 'x'
    if row[opcode_variant].value != None and len(str(row[opcode_variant].value)) == 1:
        regname = str(row[opcode_variant].value).strip().lower()

    sf = row[opcode_sf].value == '1'

    for i in range(opcode_bits_lsb, opcode_bits_msb - 1, -1):
        cell = row[i]
        if cell.value == None: continue
        value = str(cell.value).strip()
        if value == '': continue
        if value == None: continue

        def append_reg(name):
            if not (regname + name) in variant:
                variant.append(regname + name)
        def append_value(value):
            if not value in variant:
                variant.append(value)

        use_last = False
        if value.find('-') != -1 and last[i - opcode_bits_lsb]:
            use_last = True
            value = last[i - opcode_bits_lsb]

        if value.find('Rd') != -1:
            append_reg('d')
        elif value.find('Rt2') != -1:
            append_reg('d2')
        elif value.find('Rt') != -1:
            append_reg('d')
        elif value.find('Rn') != -1:
            append_reg('n')
        elif value.find('Rm') != -1:
            append_reg('m')
        elif value.find('Rs') != -1:
            append_reg('s')
        elif value.find('Ra') != -1:
            append_reg('a')
        elif value.find('CR_m') != -1:
            append_value('crm')
        elif value.find('CR_n') != -1:
            append_value('crn')
        elif value.find('op1') != -1:
            append_value('op1')
        elif value.find('op2') != -1:
            append_value('op2')
        elif value.find('shift') != -1:
            append_value('shift')
        elif value.find('S') != -1:
            append_value('s')
        elif value.find('N') != -1:
            append_value('n')
        elif value.find('L') != -1:
            append_value('l')
        elif value.find('hw') != -1:
            append_value('hw')
        elif value.find('cond') != -1:
            append_value('cond')
        elif value.find('nzcv') != -1:
            append_value('nzcv')
        elif value.find('option') != -1:
            append_value('option')
        elif re.search(r'imm\w+\b', value):
            imm_match = re.search(r'imm\w+\b', value)
            append_value(f"{imm_match.group(0)}")
        elif re.search(r'b\w+\b', value):
            imm_match = re.search(r'b\w+\b', value)
            append_value(f"{imm_match.group(0)}")
        elif re.search(r'[a-zA-Z]+', value) and row[4].value:
            print(f'skipping {row[4].value} {row[opcode_value_index].value}   {value}')
            continue

        if not re.search(r'[01-]', value) and not use_last:
            # print(f'{row[3].value} {cell.value} {str()} ')
            for j in range(calculate_span_via_ranges(sheet, cell)):
                last[i - opcode_bits_lsb + j] = value

    if len(variant) == 0:
        append_value('noarg')


    def cmp(a, b):
        return (a > b) - (a < b) 
    def vcmp(a: str, b: str):
        if b.startswith(regname + 'd'):
            return 1
        if a.startswith(regname + 'd'):
            return -1

        if b.startswith(regname + 'a'):
            return -1
        if a.startswith(regname + 'a'):
            return 1

        if b.startswith(regname + 'n'):
            return 1
        if a.startswith(regname + 'n'):
            return -1

        if b.startswith(regname + 'm'):
            return 1
        if a.startswith(regname + 'm'):
            return -1

        if b.startswith('crn'):
            return 1
        if a.startswith('crn'):
            return -1

        if b.startswith('cr'):
            return 1
        if a.startswith('cr'):
            return -1

        if b.startswith('op1'):
            return 1
        if a.startswith('op1'):
            return -1

        if b.startswith('shift'):
            return -1
        if a.startswith('shift'):
            return 1

        if b.startswith('cond'):
            return -1
        if a.startswith('cond'):
            return 1

        if b.startswith('sn'):
            return 1
        if a.startswith('sn'):
            return -1

        if b.startswith('s'):
            return -1
        if a.startswith('s'):
            return 1
            # return -cmp(a, b)
        return 0

    opcode_name = row[4].value
    if opcode_name == None:
        continue

    modifier = 0
    specific = ''
    if row[opcode_specific].value != None and len(str(row[opcode_specific].value)) < 10:
        specific = str(row[opcode_specific].value).strip()
        if specific == 'post':
            modifier |= 1
            opcode_name += '_' + specific
        elif specific == 'pre':
            modifier |= 2
            opcode_name += '_' + specific
        elif specific == 'off':
            modifier |= 4
            opcode_name += '_' + specific

    if opcode_name == 'SCVTF':
        break
    variant = sorted(variant, key=cmp_to_key(vcmp))

    if not (opcode_name in opcodes):
        opcodes[opcode_name] = {}

    if row[opcode_value_index].value == None:
        continue
    try:
        opcode_value = row[opcode_value_index].value.removeprefix('0x').upper()
        opcode_value = int(opcode_value, 16)
    except:
        continue

    str_variant = '_'.join(variant)
    if (str_variant, sf) in opcodes[opcode_name]:
        if modifier != opcodes[opcode_name][(str_variant, sf)][1]:
            old = opcodes[opcode_name][(str_variant, sf)]
            opcodes[opcode_name][(str_variant, sf)] = (old[0], old[1] | modifier)
        else:
            print(f"duplicate {opcode_name} -> {str_variant}")
        # raise Exception()
    else:
        opcodes[opcode_name][(str_variant, sf)] = (opcode_value, modifier)

    # if opcode_name == 'STP':
    # print(f"{opcode_name}   {}")

    # print(opcode_value_index)
    # print(f"    [AARCH64_OPCODE_{opcode_name}] = {{ .value = 0x{opcode_value}u }},")
if True:
    print(f"    AARCH64_OPCODE_{'MOV'},")
    for (op, variants) in opcodes.items():
        print(f"    AARCH64_OPCODE_{op},")

    for (op, variants) in opcodes.items():
        if op == 'ORR':
            print(f"    [AARCH64_OPCODE_{'MOV'}] = " + "{")
            for (variant, (value, modifier)) in variants.items():
                print(f"        .{variant[0]} = " + "{ .value = " + f"0x{value:X}u", end="")
                print(" },")
            print("    },")
            
        print(f"    [AARCH64_OPCODE_{op}] = " + "{")
        for (variant, (value, modifier)) in variants.items():
            print(f"        .{variant[0]} = " + "{ .value = " + f"0x{value:X}u", end="")
            print(" },")
        print("    },")
