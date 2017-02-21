#!/usr/bin/env groovy

def excludePatterns = [
    '.md',
    'README',
    'LICENSE',
    'Makefile',
]

node('docker && imgtec') {
    def docker_image
    docker_image = docker.image "imgtec/creator-builder:latest" // TODO for now using the creator one
    docker_image.inside {
        stage('Prepare') {
            checkout([$class: 'GitSCM',
                userRemoteConfigs: scm.userRemoteConfigs,
                branches: scm.branches,
                doGenerateSubmoduleConfigurations: scm.doGenerateSubmoduleConfigurations,
                submoduleCfg: scm.submoduleCfg,
                browser: scm.browser,
                gitTool: scm.gitTool,
                extensions: scm.extensions + [
                    [$class: 'CleanCheckout'],
                    [$class: 'PruneStaleBranch'],
                ],
            ])
            // Skip CI if changes in README, else continue building
            sh 'git show --pretty="format:" --name-only --no-merges 89ffc511f185058ac6bf3fdaa44c932227d3416e > commandResult'
            String[] result = new File('commandResult')
            echo "Git commit file(s): ${result}"
            echo "First element: ${result[0]}"
            for (pattern in excludePatterns) {
                if (result.size() == 1 && result[0] ==~ /.*$${pattern}/) {
                    echo 'Changes in README, skipping CI'
                    currentBuild.result = 'SUCCESS'
                    sh "exit 0"
                }
            }
        }
        stage('Build') {
            sh 'make CROSS_COMPILE=/opt/toolchains/mips-img-linux-gnu/2016.05-03/bin/mips-img-linux-gnu- pistachio_marduk_defconfig'
            sh 'make CROSS_COMPILE=/opt/toolchains/mips-img-linux-gnu/2016.05-03/bin/mips-img-linux-gnu-'
        }
        stage('Upload') {
            archiveArtifacts 'u-boot-pistachio*.img'
        }
    }
}
