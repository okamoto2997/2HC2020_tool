#!/usr/bin/env sh

if [ ! -d "in" ]; then
  mkdir in/
else
  echo "The all files in \"in\" will be erased. Are you sure? (y/N)"
  read reply
  if [ "y" = "$reply" ]; then
    rm -rf in/
    mkdir in/
    echo "Continue."
  else
    echo "Abort."
    exit 1
  fi
fi

echo "try to make \"generator_[AB]\""
make -s || exit 1

generator=`find -maxdepth 1 -type f -name "generator_*" | head -n1`

DAY_TYPE=0
echo "make test case for day type ${DAY_TYPE}"
for id in `seq 6`; do
  SEED=`od -vAn --width=4 -tu4 -N4 </dev/urandom | sed -e "s/^ *//"`
  ${generator} -s ${SEED} -d ${DAY_TYPE} > in/test_case_${DAY_TYPE}_${id}_${SEED}.in | exit 1
done

DAY_TYPE=1
echo "make test case for day type ${DAY_TYPE}"
for id in `seq 2`; do
  SEED=`od -vAn --width=4 -tu4 -N4 </dev/urandom | sed -e "s/^ *//"`
  ${generator} -s ${SEED} -d ${DAY_TYPE} > in/test_case_${DAY_TYPE}_${id}_${SEED}.in | exit 1
done

DAY_TYPE=2
echo "make test case for day type ${DAY_TYPE}"
for id in `seq 6`; do
  SEED=`od -vAn --width=4 -tu4 -N4 </dev/urandom | sed -e "s/^ *//"`
  ${generator} -s ${SEED} -d ${DAY_TYPE} > in/test_case_${DAY_TYPE}_${id}_${SEED}.in | exit 1
done

DAY_TYPE=3
echo "make test case for day type ${DAY_TYPE}"
for id in `seq 2`; do
  SEED=`od -vAn --width=4 -tu4 -N4 </dev/urandom | sed -e "s/^ *//"`
  ${generator} -s ${SEED} -d ${DAY_TYPE} > in/test_case_${DAY_TYPE}_${id}_${SEED}.in | exit 1
done

echo "All finished!"