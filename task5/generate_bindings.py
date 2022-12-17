import sys

max_count = int(sys.argv[1])

print(f'// Automatically generated by calling `python3 generate_bindings.py {max_count} > bindings.h`')
print('if constexpr (field_count == 0) {')
print('    return std::tuple<>();')

for field_count in range(1, max_count + 1):
    print('} else if constexpr (field_count == ' + str(field_count) + ') {')
    values = ', '.join(map(lambda x: f'v{x}', range(1, field_count + 1)))
    print(f'    auto &[{values}] = value;')
    print(f'    return std::tie({values});')

print('}')

