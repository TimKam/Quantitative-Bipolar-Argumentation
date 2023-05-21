import networkx as nx
from networkx.drawing.nx_agraph import graphviz_layout

from qbaf import QBAFramework

def visualize(qbaf: QBAFramework, explanations=[], with_fs=False, round_to=2):
    """
    Takes a QBAF and renders it as a graph
    
    Args:
            qbaf (QBAFramework): The QBAF that is supposed to be rendered
            explanations (List): List of sets of explanation arguments that are supposed to be highlighted
            with_fs (Bool): Set to `True` if final strenghts should be rendered as well.
                            Defaults to `False`.
            round_to (int): Number of decimals the strengths should be rounded to before rendering.
                            Defaults to `2`.
            
    """
    args = qbaf.arguments
    initial_strengths = qbaf.initial_strengths
    for key, value in initial_strengths.items():
        initial_strengths[key] = round(value, round_to)
    if with_fs:
        final_strengths = qbaf.final_strengths
        for key, value in final_strengths.items():
            final_strengths[key] = round(value, round_to)
    graph = nx.DiGraph()
    nodes = {}
    edge_colors = []
    node_sizes = []
    for arg in args:
        if with_fs:
            label = f'{arg} ({initial_strengths[arg]}): {final_strengths[arg]}'
        else:
            label = f'{arg} ({initial_strengths[arg]})'
        graph.add_node(label)
        nodes[arg] = label
        edge_color = '#fff'
        node_size = 2000
        for explanation in explanations:
            if arg in explanation:
                edge_color = '#af4154'
                node_size = 3000
        node_sizes.append(node_size)
        edge_colors.append(edge_color)
    labels = {}
    for att in qbaf.attack_relations.relations:
        labels[(nodes[att[0]], nodes[att[1]])] = '-'
        graph.add_edge(nodes[att[0]], nodes[att[1]])
    for supp in qbaf.support_relations.relations:
        labels[(nodes[supp[0]], nodes[supp[1]])] = '+'
        graph.add_edge(nodes[supp[0]], nodes[supp[1]])
    pos = graphviz_layout(graph, prog='dot')
    nx.draw(graph, pos, with_labels=True, node_shape='8', node_size=node_sizes, font_size=8, node_color='#fff', edgecolors=edge_colors,bbox=dict(facecolor='black', alpha=0.1))
    nx.draw_networkx_edge_labels(
    graph, pos,
    edge_labels=labels,
    font_size=25,
    rotate=False)