import qbaf_python.argument
import qbaf

def main():
    c_arg = qbaf.QBAFArgument('a', 'desc', 0.5)
    py_arg = qbaf_python.argument.QBAFArgument('a', 'desc', 0.5)

    for arg in [c_arg, py_arg]:
        print(arg)
        print(f'{arg.name=}')
        print(f'{arg.description=}')
        print(f'{arg.initial_weight=}')
        print(f'{arg.final_weight=}')

if __name__ == '__main__':
    main()