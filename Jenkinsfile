pipeline {
  agent any
  stages {
    stage('Build') {
      steps {
        sh 'make'
      }
    }
    stage('Compile test/Test.hs') {
      steps {
        sh './schc test/Test.hs'
      }
    }
  }
}