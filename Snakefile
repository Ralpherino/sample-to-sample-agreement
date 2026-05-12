configfile: "config/config.yaml"

SAMPLE1_FILE = config["sample1_file"]
SAMPLE2_FILE = config["sample2_file"]

SAMPLE1_TREE = config["sample1_tree"]
SAMPLE2_TREE = config["sample2_tree"]

SAMPLE1_LABEL = config["sample1_label"]
SAMPLE2_LABEL = config["sample2_label"]

NORMALIZE = config["normalize"]

BRANCHES = list(config["branches"].keys())
CUTS = list(config["cuts"].keys())


rule all:
    input:
        expand(
            "mc_data_comparison/plots/{branch}/{cut}/{branch}_MC_DATA_{cut}.png",
            branch=BRANCHES,
            cut=CUTS
        )


rule get_range:
    output:
        "mc_data_comparison/ranges/{branch}/{cut}.txt"
    params:
        sample1_file=SAMPLE1_FILE,
        sample2_file=SAMPLE2_FILE,
        sample1_tree=SAMPLE1_TREE,
        sample2_tree=SAMPLE2_TREE,
        cut=lambda wildcards: config["cuts"][wildcards.cut]
    shell:
        r"""
        mkdir -p $(dirname {output})

        root -l -b -q 'scripts/get_range.C(
            "{params.sample1_file}",
            "{params.sample2_file}",
            "{params.sample1_tree}",
            "{params.sample2_tree}",
            "{wildcards.branch}",
            "{params.cut}",
            "{output}"
        )'
        """


rule make_mc_hist:
    input:
        range_file="mc_data_comparison/ranges/{branch}/{cut}.txt"
    output:
        "mc_data_comparison/hists/{branch}/{cut}/sample1.root"
    params:
        input_file=SAMPLE1_FILE,
        tree=SAMPLE1_TREE,
        bins=lambda wildcards: config["branches"][wildcards.branch]["bins"],
        cut=lambda wildcards: config["cuts"][wildcards.cut]
    shell:
        r"""
        mkdir -p $(dirname {output})

        root -l -b -q 'scripts/make_hist.C(
            "MC",
            "{params.input_file}",
            "{params.tree}",
            "{wildcards.branch}",
            "{params.cut}",
            {params.bins},
            "{input.range_file}",
            "{output}"
        )'
        """


rule make_data_hist:
    input:
        range_file="mc_data_comparison/ranges/{branch}/{cut}.txt"
    output:
        "mc_data_comparison/hists/{branch}/{cut}/sample2.root"
    params:
        input_file=SAMPLE2_FILE,
        tree=SAMPLE2_TREE,
        bins=lambda wildcards: config["branches"][wildcards.branch]["bins"],
        cut=lambda wildcards: config["cuts"][wildcards.cut]
    shell:
        r"""
        mkdir -p $(dirname {output})

        root -l -b -q 'scripts/make_hist.C(
            "DATA",
            "{params.input_file}",
            "{params.tree}",
            "{wildcards.branch}",
            "{params.cut}",
            {params.bins},
            "{input.range_file}",
            "{output}"
        )'
        """


rule plot_mc_data:
    input:
        mc_hist="mc_data_comparison/hists/{branch}/{cut}/sample1.root",
        data_hist="mc_data_comparison/hists/{branch}/{cut}/sample2.root"
    output:
        png="mc_data_comparison/plots/{branch}/{cut}/{branch}_MC_DATA_{cut}.png"
    params:
        sample1_label=SAMPLE1_LABEL,
        sample2_label=SAMPLE2_LABEL,
        cut=lambda wildcards: config["cuts"][wildcards.cut],
        normalize="true" if NORMALIZE else "false"
    shell:
        r"""
        mkdir -p $(dirname {output.png})

        root -l -b -q 'scripts/plot_mc_data.C(
            "{input.mc_hist}",
            "{input.data_hist}",
            "{wildcards.branch}",
            "{params.sample1_label}",
            "{params.sample2_label}",
            "{wildcards.cut}",
            "{params.cut}",
            {params.normalize},
            "{output.png}"
        )'
        """
