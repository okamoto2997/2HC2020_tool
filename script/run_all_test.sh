#!/usr/bin/env sh

if [ -z $1 ]; then
  echo "Usage: $0 answer_exec"
  exit 1
fi

echo "Try to make \"judge_[AB]\"."
make -s || exit 1
echo "Successed."

answer=$1
result_dir=result_`date "+%Y%m%d_%H%M%S"`
summary=${result_dir}/summary
judge=`find -maxdepth 1 -type f -name "judge_*" | head -n1`

mkdir -p ${result_dir}
find in/ -maxdepth 1 -type f \
  | sort \
  | while read file; do
  out=${result_dir}/`basename $file`
  echo "run test case \"$file\" and store result to \"$out\""
  ${judge} ${answer} < $file > $out || exit 1
  sed -i -e "s/^$//" $out
  printf "%s: %s\n" $file `cat $out` >> ${summary}
done

echo >> ${summary}
total_score=`cat ${result_dir}/* | awk 'BEGIN{sum=0}{sum+=$1}END{print sum}'`
printf 'total score: %d' ${total_score} >> ${summary}

echo "All results are summurized in \"${summary}\"."
echo "finished."

