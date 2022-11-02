from qbaf_python import QBAFArgument as Arg
from qbaf_python import QBAFARelations as Relations

def main():
    #a,b,c,d = Arg('a'), Arg('b'), Arg('c'), Arg('d')
    a,b,c,d = 'a', 'b', 'c', 'd'
    relations_list = [(a,b), (a,c), (d,c)]
    rels = Relations(relations_list)

    print("rels=[ ", end='')
    for relation in rels.relations:
        p, q = relation
        print(f'({p}, {q})', end=' ')
    print("]")
    
    print(f'{rels.contains(a,b)=}')
    print(f'{rels.contains(b,a)=}')

    print(f'{rels.patients(a)=}')

    print(f'{rels.agents(a)=}')

    print("Remove (a,b)")
    rels.remove(a,b)

    print("rels=[ ", end='')
    for relation in rels.relations:
        p, q = relation
        print(f'({p}, {q})', end=' ')
    print("]")

    print(f'{rels.contains(a,b)=}')
    print(f'{rels.contains(b,a)=}')

    print(f'{rels.patients(a)=}')

    print(f'{rels.agents(a)=}')

    print("Add (b,a)")
    rels.add(b,a)

    print("rels=[ ", end='')
    for relation in rels.relations:
        p, q = relation
        print(f'({p}, {q})', end=' ')
    print("]")

    print(f'{rels.contains(a,b)=}')
    print(f'{rels.contains(b,a)=}')

    print(f'{rels.patients(a)=}')

    print(f'{rels.agents(a)=}')

if __name__ == '__main__':
    main()