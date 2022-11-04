from qbaf import QBAFArgument as Arg
from qbaf import QBAFARelations as Relations

def main():
    a,b,c,d = Arg('a'), Arg('b'), Arg('c'), Arg('d')
    relations_list = [(a,b), (a,c), (d,c)]
    rels = Relations(relations_list)

    print(f'{rels=}')
    
    print(f'{rels.contains(a,b)=}')
    print(f'{rels.contains(b,a)=}')

    print(f'{rels.patients(a)=}')

    print(f'{rels.agents(a)=}')

    print("Remove (a,b)")
    rels.remove(a,b)

    print(f'{rels.relations=}')

    print(f'{rels.contains(a,b)=}')
    print(f'{rels.contains(b,a)=}')

    print(f'{rels.patients(a)=}')

    print(f'{rels.agents(a)=}')

    print("Add (b,a)")
    rels.add(b,a)

    print(f'{rels.relations=}')

    print(f'{rels.contains(a,b)=}')
    print(f'{rels.contains(b,a)=}')

    print(f'{rels.patients(a)=}')

    print(f'{rels.agents(a)=}')

if __name__ == '__main__':
    main()