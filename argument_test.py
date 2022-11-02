import qbaf_python.argument
import qbaf

def main():
    c_args = [qbaf.QBAFArgument('a', 'desc'),
                qbaf.QBAFArgument('a'),
                qbaf.QBAFArgument('b', 'desc')]
    py_args = [qbaf_python.argument.QBAFArgument('a', 'desc'),
                qbaf_python.argument.QBAFArgument('a'),
                qbaf_python.argument.QBAFArgument('b', 'desc')]

    for args in [c_args, py_args]:
        for arg in args:
            print(arg)
            print(f'{arg.name=}')
            print(f'{arg.description=}')
        arg0, arg1, arg2 = args
        print(f'{arg0}=={arg1}: {arg0==arg1}')
        print(f'{arg0}=={arg2}: {arg0==arg2}')
        print(f'{arg1}=={arg2}: {arg1==arg2}')
        print()
        print()


if __name__ == '__main__':
    main()